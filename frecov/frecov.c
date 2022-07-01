#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// Copied from the manual
struct fat32hdr {
  u8  BS_jmpBoot[3];
  u8  BS_OEMName[8];
  u16 BPB_BytsPerSec; // bytes per sector
  u8  BPB_SecPerClus; // sectors per cluster
  u16 BPB_RsvdSecCnt; // reserved sectors in the reserved region
  u8  BPB_NumFATs;    // count of FATs
  u16 BPB_RootEntCnt; // count of 32-bytes directory entry for FAT32 this is 0
  u16 BPB_TotSec16;   // for FAT32 this is 0
  u8  BPB_Media;      // specific legal value for media: 0xF8 for fixed media
  u16 BPB_FATSz16;    // for FAT32 this is 0
  u16 BPB_SecPerTrk;  // sectors per track for interrupt 0x13
  u16 BPB_NumHeads;   // number of heads for interrupt 0x13
  u32 BPB_HiddSec;    // hidden sectors
  u32 BPB_TotSec32;   // the count of all sectors
  u32 BPB_FATSz32;    // count if sectors occupied by one FAT
  u16 BPB_ExtFlags;   // status flags
  u16 BPB_FSVer;      // version number, set to 0x0
  u32 BPB_RootClus;   // cluster number of the first cluster of the root directory
  u16 BPB_FSInfo;     // sector number of FSINFO in the reserved area
  u16 BPB_BkBootSec;  // sector number in the reserved area of the copy of the boot record
  u8  BPB_Reserved[12]; // reserved ,set to 0x0
  u8  BS_DrvNum;      // interrupt 0x13 drive number
  u8  BS_Reserved1;   // reserved, set to 0x0
  u8  BS_BootSig;     
  u32 BS_VolID;
  u8  BS_VolLab[11];
  u8  BS_FilSysType[8]; // string "FAT"
  u8  __padding_1[420]; // set to 0x0
  u16 Signature_word; // 0x55
} __attribute__((packed));


typedef struct fat32dent
{
  u8 DIR_Name[11];
  u8 DIR_Attr;
  u8 DIR_NTRes;
  u8 DIR_CtrTimeTenth;
  u16 DIR_CrtTime;
  u16 DIR_CrtDate;
  u16 DIR_LstAccDate;
  u16 DIR_FstClusHI;
  u16 DIR_WrtTime;
  u16 DIR_WrtDate;
  u16 DIR_FstClusLO;
  u32 DIR_FileSize; /* data */
}__attribute__((packed)) fat32dent ;

typedef struct fat32Ldent
{
  u8 LDIR_Ord;
  u16 LDIR_Name1[5];
  u8 LDIR_Attr;
  u8 LDIR_Type;
  u8 LDIR_Chksum;
  u16 LDIR_Name2[6];
  u16 LDIR_FstClusLO;
  u16 LDIR_Name3[2];
} __attribute__((packed)) fat32Ldent ;


#define CLUS_INVALID   0xffffff7

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

off_t size;
struct fat32hdr *hdr = NULL;
size_t BytesPerClus;

void *map_disk(const char *fname);

void *cluster_to_sec(int n, struct fat32hdr *hdr); // get the beginning sector address of Nth cluster.

u_int Get_Data_Clus(struct fat32hdr *hdr);

void Traverse_Dir(fat32dent *dent); // traverse a directory-like cluster

void file_recovery(fat32dent *dent, char *name, int size);     // recover a bmp file

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
    exit(1);
  }

  setbuf(stdout, NULL);

  assert(sizeof(struct fat32hdr) == 512); // defensive
  assert(sizeof(fat32dent) == 32);
  assert(sizeof(fat32Ldent) == 32);
  // map disk image to memory
  hdr = map_disk(argv[1]);
  BytesPerClus = hdr -> BPB_BytsPerSec * hdr -> BPB_SecPerClus;
  // FAT table:
  uintptr_t FATentry = (uintptr_t)hdr + (uintptr_t)(hdr -> BPB_RsvdSecCnt * hdr -> BPB_BytsPerSec);
  u32 *FAT = (u32 *)FATentry;
  // Data Region:
  uintptr_t DataEntry = (uintptr_t)FATentry + (uintptr_t)(hdr -> BPB_NumFATs * hdr -> BPB_FATSz32 * hdr -> BPB_BytsPerSec);
  u_int DataClusNum = Get_Data_Clus(hdr); // get the number of clusters in data region;
  // traverse every cluster to judge if it's a cluster of directory:
  // for every sector, there are less than 16 entries; 
  // thus for every cluster, there are less than 8 * 16 = 128 entries 
  for(int i = 2; i < DataClusNum + 2; i++)
  { 
    int sup = 0;
    fat32dent *entry = (fat32dent *)cluster_to_sec(i, hdr);
    fat32dent *dir = entry;
    for(int j = 0; j < 8 * 16; j++){
    if(entry -> DIR_Attr == ATTR_LONG_NAME || entry -> DIR_Attr == ATTR_ARCHIVE || entry -> DIR_Attr == ATTR_DIRECTORY || entry -> DIR_Attr == ATTR_READ_ONLY)
      sup ++; // SUP reflect how we trust this is a directory entry;
    entry = entry + 1;
    }
    if(sup >= 32) Traverse_Dir(dir); // This is considered as a directory.
  }
   munmap(hdr, hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec);
}

void Traverse_Dir(fat32dent *dent) {
  uintptr_t ClusEnd = (uintptr_t)dent + (uintptr_t)(hdr -> BPB_BytsPerSec * hdr -> BPB_SecPerClus);
  fat32dent *CurDir = dent;
  fat32Ldent *CurLDir = (fat32Ldent *)dent;
  char buf[256];
  while((uintptr_t)CurDir <= ClusEnd && (uintptr_t)CurLDir <= ClusEnd) {
    memset(buf, 0, sizeof(buf));
    int size = 0;
    if(CurDir -> DIR_Attr == ATTR_LONG_NAME) // Long name directory;
    {
      u32 LdentNum = (CurLDir -> LDIR_Ord) & (0x0f); // number of long name entry
      CurLDir = CurLDir + LdentNum;
      if((uintptr_t)CurLDir > ClusEnd) break; // rejudge
      CurDir = (fat32dent *)CurLDir;
      CurLDir = CurLDir - 1;
      while(LdentNum --){
        int flag = 0; // indicates whether long name has been recorded over
        // name 1 copying:
        for(int i = 0; i < 5; i++){
            if(!(CurLDir -> LDIR_Name1[i] == 0xffff)){
              u8 CurChar = (u8) (CurLDir -> LDIR_Name1[i]);
              buf[size] = (char)CurChar;
              size ++;
            }
           else{ flag = 1;
                  break;
           }
        }
        if(flag) break;
        // name 2 copying:
        for(int i = 0; i < 6; i++){
          if(!(CurLDir -> LDIR_Name2[i] == 0xffff)){
            u8 CurChar = (u8) (CurLDir -> LDIR_Name2[i]);
            buf[size] = (char)CurChar;
            size ++;
          }
          else { flag = 1;
                 break;
          }
        }
        if(flag) break;
        // name 3 copying:
        for(int i = 0; i < 2; i++){
          if(!(CurLDir -> LDIR_Name3[i] == 0xffff)){
            u8 CurChar = (u8) (CurLDir -> LDIR_Name3[i]);
            buf[size] = (char)CurChar;
            size ++;
          }
          else { flag = 1;
                 break;
          }
        }
        if(flag) break;
        CurLDir = CurLDir - 1;
      }
      //assert(LdentNum == -1);
    }
    else {
      for(int i = 0; i < sizeof(CurDir -> DIR_Name); i++) {
        if(CurDir -> DIR_Name[i] != ' '){
          if(i == 8) buf[size++] = '.';
          buf[size++] = CurDir -> DIR_Name[i];
        }
      }
    }
    //printf("name is %s\n", buf);
    if(CurDir -> DIR_Attr == ATTR_ARCHIVE){ 
      file_recovery(CurDir, buf, size);
    }
    CurDir = CurDir + 1;
    CurLDir = (fat32Ldent *)CurDir;
    
    
}
}

void file_recovery(fat32dent *dent, char *name, int size) {
  u32 dataClus = (dent -> DIR_FstClusLO) | ((dent -> DIR_FstClusHI) << 16);
  u32 filesize = dent -> DIR_FileSize;
  u32 fclusnum = (filesize % BytesPerClus) ? filesize / BytesPerClus + 1 : filesize / BytesPerClus;
  char prefix[10] ="/tmp/";
  char filename[128];
  sprintf(filename, "%s%s", prefix, name);
  FILE *fp;
  fp = fopen(filename, "a+");
  char databuf[4096];
  void *ClusBegin = (void *)cluster_to_sec(dataClus, hdr);
  memcpy(databuf, ClusBegin, BytesPerClus);
  fprintf(fp, "%s", databuf);
  fclose(fp);
  // sha1num :
  FILE *fm;
  char commands[128] = "sha1sum ";
  char ansbuf[256];
  strcat(commands, filename);
  fm = popen(commands, "r");
  assert(fm >= 0);
  fscanf(fm, "%s", ansbuf);
  pclose(fm);
  printf("%s ", ansbuf);
  printf("%s\n", name);
  return ;
}
  // dcim = (fat32dent *)cluster_to_sec(dataClus, hdr);
  // uintptr_t ClusEnd = (uintptr_t)dcim + (uintptr_t)(hdr -> BPB_BytsPerSec * hdr -> BPB_SecPerClus);
  // // start of bmp files:
  // // if Long name dent, attr must be 0Fh:
  // fat32dent *CurDir = dcim;
  // fat32Ldent *CurLDir = (fat32Ldent *)dcim;
  // while((uintptr_t)CurDir <= ClusEnd && (uintptr_t)CurLDir <= ClusEnd) {
  //   char buf[256];
  //   int size = 0;
  //   if(CurDir -> DIR_Attr == ATTR_LONG_NAME) // Long name directory;
  //   {
  //     u32 LdentNum = (CurLDir -> LDIR_Ord) & (0x0f); // number of long name entry
  //     CurLDir = CurLDir + LdentNum;
  //     CurDir = (fat32dent *)CurLDir;
  //     CurLDir = CurLDir - 1;
  //     while(LdentNum --){
  //       int flag = 0; // indicates whether long name has been recorded over
  //       // name 1 copying:
  //       for(int i = 0; i < 5; i++){
  //           if(!(CurLDir -> LDIR_Name1[i] == 0xffff)){
  //             u8 CurChar = (u8) (CurLDir -> LDIR_Name1[i]);
  //             buf[size] = (char)CurChar;
  //             size ++;
  //           }
  //          else{ flag = 1;
  //                 break;
  //          }
  //       }
  //       if(flag) break;
  //       // name 2 copying:
  //       for(int i = 0; i < 6; i++){
  //         if(!(CurLDir -> LDIR_Name2[i] == 0xffff)){
  //           u8 CurChar = (u8) (CurLDir -> LDIR_Name2[i]);
  //           buf[size] = (char)CurChar;
  //           size ++;
  //         }
  //         else { flag = 1;
  //                break;
  //         }
  //       }
  //       if(flag) break;
  //       // name 3 copying:
  //       for(int i = 0; i < 2; i++){
  //         if(!(CurLDir -> LDIR_Name3[i] == 0xffff)){
  //           u8 CurChar = (u8) (CurLDir -> LDIR_Name3[i]);
  //           buf[size] = (char)CurChar;
  //           size ++;
  //         }
  //         else { flag = 1;
  //                break;
  //         }
  //       }
  //       if(flag) break;
  //       CurLDir = CurLDir - 1;
  //     }
  //     //assert(LdentNum == -1);
  //   }
  //   else {
  //     for(int i = 0; i < sizeof(CurDir -> DIR_Name); i++) {
  //       if(CurDir -> DIR_Name[i] != ' '){
  //         if(i == 8) buf[size++] = '.';
  //         buf[size++] = CurDir -> DIR_Name[i];
  //       }
  //     }
  //   }
  
  //   CurDir = CurDir + 1;
  //   CurLDir = (fat32Ldent *)CurDir;
  //   for(int i = 0; i < size; i++)
  //     putchar(buf[i]);
  //   printf("\n");
  // TODO: frecov
  
void *map_disk(const char *fname) {
  int fd = open(fname, O_RDWR);

  if (fd < 0) {
    perror(fname);
    goto release;
  }

  size = lseek(fd, 0, SEEK_END);
  if (size == -1) {
    perror(fname);
    goto release;
  }

  struct fat32hdr *hdr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (hdr == (void *)-1) {
    goto release;
  }

  close(fd);

  if (hdr->Signature_word != 0xaa55 ||
      hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec != size) {
    fprintf(stderr, "%s: Not a FAT file image\n", fname);
    goto release;
  }
  return hdr;

release:
  if (fd > 0) {
    close(fd);
  }
  exit(1);
}

void *cluster_to_sec(int n, struct fat32hdr *hdr) {
  u32 DataSec = hdr -> BPB_RsvdSecCnt + hdr->BPB_NumFATs * hdr -> BPB_FATSz32; // getting the number of sector before data region;
  DataSec += (n - 2) * hdr -> BPB_SecPerClus;
  return ((char *)hdr) + DataSec * hdr -> BPB_BytsPerSec;
}

u_int Get_Data_Clus (struct fat32hdr *hdr) {
  u32 Datasec = hdr -> BPB_RsvdSecCnt + hdr -> BPB_FATSz32 * hdr -> BPB_NumFATs;
  u_int ans = (size - Datasec * hdr -> BPB_BytsPerSec) / (hdr -> BPB_BytsPerSec * hdr -> BPB_SecPerClus);
  return ans;
}//   // FAT recovery:
  //   if(CurDir -> DIR_Attr != ATTR_DIRECTORY && CurDir -> DIR_Attr != ATTR_LONG_NAME){
  //     size_t fsz = CurDir -> DIR_FileSize;
  //     u_int ClusNum = (fsz / BytesPerClus) + 1;
  //     u_int Clusid = (CurDir -> DIR_FstClusLO) | ((CurDir -> DIR_FstClusHI) << 16);
  //     // assuming continuous cluster
  //     while(ClusNum --){
  //         if(ClusNum) FAT[Clusid] = (u32)(Clusid + 1);
  //         else FAT[Clusid] = 0xffffffff;
  //         Clusid = FAT[Clusid];
  //     }
  // } 