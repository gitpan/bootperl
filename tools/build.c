#include <stdlib.h>
#include <stdio.h> /* buffered!! (file code isn't) */
#include <stdint.h> /* lets be portable */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// 18K chunks (CHUCKS * 512)
//#define CHUNKS 36
#define CHUNKS 36

#define min(a,b) (((a) < (b)) ? (a) : (b))

typedef u_int8_t byte;
typedef u_int16_t word;
typedef u_int32_t dword;
typedef u_int64_t quad;

struct sect_addr {
  byte sector, track; /* cx */
  byte device, head; /* dx */
  byte num_sect; /* al */
};
#define SECT_ADDR(x) ((struct sect_addr *)(x))

const unsigned int SPT = 18; /* Sectors per track */
const unsigned int TPH = 80; /* Tracks per head (side) (This is now
                                unused it was the result of a
                                misunderstanding about the CHS order,
                                I thought it was HCS!!) */
const unsigned int HEADS = 2;

__inline__ void inc_sector(struct sect_addr *addr, byte count)
{ /* version two of this see comment above about mistake */
  /* order: track head sector
   * track: 0-79
   * head: 0-1
   * sector: 1-18 */
  
  /* old code does not do sector right
  addr->sector += count;
  addr->head += addr->sector / SPT;
  addr->sector = addr->sector % SPT;
  addr->track += addr->head / HEADS;
  addr->head = addr->head % HEADS; */

  addr->sector += count;
  if (addr->sector > SPT) {
    addr->sector--; /* cheat */

    addr->head += addr->sector / SPT;
    addr->sector = addr->sector % SPT;
    addr->track += addr->head / HEADS;
    addr->head = addr->head % HEADS;

    addr->sector++; /* cheat */
  }
}

__inline__ void print_sector(const struct sect_addr *addr)
{ /* reordered output to make mistake clearer */
  fprintf(stderr, "Device %i, Track %i, Head %i, Sector %i, Count %i\n",
	  addr->device, addr->track, addr->head, addr->sector,
	  addr->num_sect);
}

struct bsect_head {
  char jmp[4]; /* 3 bytes cli + jmp past header + 1 byte extra */
  char signature[4];
  word id;
  word check;
  word version;
  byte flag;
  byte pad1; /* unused */
  struct sect_addr secaddr[10];
  byte pad2[4]; /* always 0 */
};
#define BSECT_HEAD(x) ((struct bsect_head *)(x))

struct second_head {
  char jmp[4]; /* 2 bytes jmp past header + 2 bytes extra */
  char signature[4];
  word id;
  word check;
  word version;
  byte flag;
  byte pad1; /* unused */
  struct sect_addr mapaddr[2];
};
#define SECOND_HEAD(x) ((struct second_head *)(x))

int main(int argc, const char ** argv)
{
  byte *bsect = NULL;
  byte *second = NULL;
  byte *map = NULL;
  byte *kernel = NULL;
  int fd;
  ssize_t sizeread;
  unsigned int secondsize, mapsize, kernelsize;
  struct stat statbuf;
  unsigned int i, j;
  struct sect_addr nextsector;

  if (argc != 4) {
    fprintf(stderr, "Usage: build bootsect second kernel >image\n");
    exit(EXIT_FAILURE);
  }

  /* Load up bootsect img */
  fd = open(argv[1], O_RDONLY);
  bsect = malloc(512);
  memset(bsect, 0, 512);
  sizeread = read(fd, bsect, 512);
  if (sizeread < 512) {
    fprintf(stderr, "bootsect image is < 512 (warn)\n");
  }
  close(fd);
  /* verify bootsect img */
  if ((strncmp(BSECT_HEAD(bsect)->signature, "BOOT", 4) != 0) ||
      (BSECT_HEAD(bsect)->id != 0x0001) ||
      (BSECT_HEAD(bsect)->check != 0x1234) ||
      (BSECT_HEAD(bsect)->version != 0x0001)) {
    fprintf(stderr, "Invalid bootsect image\n");
    goto cleanup;
  }
  /* clear halt flag */
  BSECT_HEAD(bsect)->flag &= ~0x01;

  /* load up second image */
  fd = open(argv[2], O_RDONLY);
  fstat(fd, &statbuf);
  /* check size (max = 8K) */
  if (statbuf.st_size > (1024 * 8)) {
    fprintf(stderr, "second image too big\n");
    goto cleanup;
  }
  /* round size in bytes up to whole sectors */
  secondsize = ((int)(statbuf.st_size / 512) + 1) * 512;
  second = malloc(secondsize);
  memset(second, 0, secondsize);
  read(fd, second, statbuf.st_size);
  close(fd);
  /* verify second img */
  if ((strncmp(SECOND_HEAD(second)->signature, "BOOT", 4) != 0) ||
      (SECOND_HEAD(second)->id != 0x0002) ||
      (SECOND_HEAD(second)->check != 0x1234) ||
      (SECOND_HEAD(second)->version != 0x0001)) {
    fprintf(stderr, "Invalid second image\n");
    goto cleanup;
  }

  /* initialise map memory */
  mapsize = /*512*/ 1024;
  map = malloc(mapsize);
  memset(map, 0, mapsize);

  /* Load up kernel image and patch map */
  fd = open(argv[3], O_RDONLY);
  fstat(fd, &statbuf);
  /* round size in bytes up to whole sectors */
  kernelsize = ((int)(statbuf.st_size / 512) + 1) * 512;
  kernel = malloc(kernelsize);
  memset(kernel, 0, kernelsize);
  read(fd, kernel, statbuf.st_size);
  close(fd);

  nextsector.sector = 2; /* first sector used by bootsect */
  nextsector.track = 0;
  nextsector.head = 0;
  nextsector.device = 0x00;
  nextsector.num_sect = 0; /* purposefully nonsensical */
  /* patch bsect (load second in 2K chunks) */
  for (i = 0, j = 0; i < (secondsize / 512); i += CHUNKS, j++) {
    BSECT_HEAD(bsect)->secaddr[j] = nextsector;
    BSECT_HEAD(bsect)->secaddr[j].num_sect = min((secondsize / 512) - i, CHUNKS);
    inc_sector(&nextsector, BSECT_HEAD(bsect)->secaddr[j].num_sect);
  }

  /* patch second for map */
  SECOND_HEAD(second)->mapaddr[0] = nextsector;
  SECOND_HEAD(second)->mapaddr[0].num_sect = mapsize / 512;
  inc_sector(&nextsector, 1);

  fprintf(stderr, "Patching map for kernel\n");
  /* patch map for kernel (load kernel in 2K chunks) */
  for (i = 0, j = 0; i < (kernelsize / 512); j++) {
    int chunks = CHUNKS;
    int remaining = HEADS * SPT - (SPT * nextsector.head + nextsector.sector)
      + 1;

    fprintf(stderr, "sector=%d spt=%d remaining=%d\n",
      nextsector.sector, SPT, remaining);
    if (remaining < chunks)
      chunks = remaining;

    SECT_ADDR(map)[j] = nextsector;
    SECT_ADDR(map)[j].num_sect = min((kernelsize / 512) - i, chunks);

    fprintf(stderr, "%d: ", j);
    print_sector(&SECT_ADDR(map)[j]);

    i += chunks;

    inc_sector(&nextsector, SECT_ADDR(map)[j].num_sect);
  }

  /* output the final image */
  write(1, bsect, 512);
  write(1, second, secondsize);
  write(1, map, mapsize);
  write(1, kernel, kernelsize);

 cleanup:
  /* free mem */
  free(map);
  free(second);
  free(bsect);

  exit(EXIT_SUCCESS);
}
