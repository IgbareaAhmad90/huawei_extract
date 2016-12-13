#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

typedef struct __attribute__((packed)){
  uint32_t signature;
  uint32_t header_len;
  uint32_t const1; /*0x00000001*/
  uint64_t hardware;
  uint32_t file_seq;
  uint32_t file_len;
  char file_date[16];
  char file_time[16];
  char word_input[16];
  uint8_t blank1[16];
  uint16_t header_checksum;
  uint16_t const2; /*0x1000*/
  uint8_t blank2[2];
  uint8_t file_checksum[];
}blob_header;

typedef struct __attribute__((packed)){
  uint8_t pad[92];
}file_header;


const char* cvt_name(uint32_t x){
  switch(x){
  case 0:
    return "system.img";
  case 0x70000000UL:
    return "cust.img";
  case 0x30000000UL:
    return "userdata.img";
  case 0x40000000UL:
    return "recovery.img";
  case 0x44000000UL:
    return "erecovery.img";
  case 0x50000000UL:
    return "cache.img";
  case 0xc1000000UL:
    return "firmware.img";
  case 0xc3000000UL:
    return "tz.img";
  case 0xfc000000UL:
    return "boot.img";
  case 0xfe000000UL:
    return "signature.img";
  case 0xff000000UL:
    return "checksum.img";
  default:
    return NULL;
  }
}

int main(int argc, char** argv){
  FILE* f = fopen(argv[1], "r");
  assert(f);
  fseek(f, 0, SEEK_END);
  long int size = ftell(f);
  assert(size > 0);
  fseek(f, 0, SEEK_SET);
  file_header *p = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fileno(f), 0);
  assert(p);
  for(blob_header *b = (blob_header*)(p + 1);
      (char*)b < (char*)p + size;
      b = ({
           char* padding;
          for(padding = (char*)b + b->header_len + b->file_len; padding < (char*)p + size && !*padding; ++padding);
          (blob_header*)padding;
          })){
    assert(b->signature == '\xA5\x5A\xAA\x55');
    assert(b->const1 == 0x00000001);
    printf("file_seq= %lx, hardware= %llx, date= %s, time= %s\n", (long)b->file_seq, (long long)b->hardware, b->file_date, b->file_time);
    char name[8 + 1];
    const char* h_name = cvt_name(b->file_seq);
    if(!h_name){
      sprintf(name, "%lx", (long)b->file_seq);
      h_name = name;
    }
    FILE* g = fopen(h_name, "w+");
    assert(g);
    size_t cnt = fwrite((char*)b + b->header_len, b->file_len, 1, g);
    assert(cnt == 1);
    fclose(g);
  }
}
