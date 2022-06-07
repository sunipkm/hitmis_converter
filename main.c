#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>
#include <fitsio.h>

#define eprintf(str, ...) \
{ \
    fprintf(stderr, "[%s, %d] " str, __func__, __LINE__, ##__VA_ARGS__); \
    fflush(stderr); \
}

#define eprintlf(str, ...) eprintf(str "\n", ##__VA_ARGS__)

typedef union
{
    struct __attribute__((packed))
    {
        uint16_t xdim; // 0-1
        uint16_t ydim; // 2-3
        char sitename[5]; // 4-8
        char unused0[10]; // 9-18
        char filename[12]; // 19-30
        uint16_t year; // 31-32
        char month[3]; // 33-35
        uint16_t day; // 36-37
        char filter[5]; // 38-42
        char unused1[2]; // 43-44
        uint16_t xbin; // 45-46
        uint16_t ybin; // 47-48
        uint16_t numsu; // 49-50
        uint16_t left; // 51-52
        uint16_t right; // 53-54
        uint16_t bottom; // 55-56
        uint16_t top; // 57-58
        char unused2[16]; // 59-74
        uint16_t numco; // 75-76
        char unused3[4]; // 77-80
        float exposure; // 81-84
        float waittime; // 85-88
        float temperature; // 89-92
        char unused4[35]; // 93-127
    };
    uint8_t bytes[128];
} hitmis_header;

void print_header(hitmis_header *h)
{
    printf("X Dim: %u | Y Dim: %u\n", h->xdim, h->ydim);
    
    printf("Site Name: ");
    for (int i = 0; i < sizeof(h->sitename); i++)
        printf("%c", h->sitename[i]);
    printf("\n");
    
    printf("File Name: ");
    for (int i = 0; i < sizeof(h->filename); i++)
        printf("%c", h->filename[i]);
    printf("\n");
    
    printf("Year: %u, Month: %c%c%c, Day: %u\n", h->year, h->month[0], h->month[1], h->month[2], h->day);
    
    printf("Filter: ");
    for (int i = 0; i < sizeof(h->filter); i++)
        printf("%c", h->filter[i]);
    printf("\n");

    printf("Bin X: %u | Bin Y: %u\n", h->xbin, h->ybin);

    printf("Left: %u | Right: %u\n", h->left, h->right);

    printf("Top: %u | Bottom: %u\n", h->top, h->bottom);

    printf("Coadd: %u\n", h->numco);

    printf("Exposure: %f s | Cadence: %f s\n", h->exposure, h->waittime);

    printf("Temperature: %f C\n", h->temperature);
}

static inline ssize_t get_fsize(char *fname)
{
    if (fname == NULL)
        return -1;
    struct stat st;
    int ret = stat(fname, &st);
    if (ret == -1)
    {
        eprintlf("Error: %s", strerror(errno));
        return -1;
    }
    return st.st_size;
}

bool savefit(char *fname, hitmis_header *hdr, uint16_t *data)
{
    if (fname == NULL)
    {
        eprintlf("File name is NULL");
        return false;
    }
    char fitsname[266];
    snprintf(fitsname, sizeof(fitsname), "%s.fit", fname);
    unlink(fitsname);
    snprintf(fitsname, sizeof(fitsname), "%s.fit[compress]", fname);
    fitsfile *fptr; 
    int status = 0;
    int bitpix = USHORT_IMG, naxis = 2;
    int bzero = 32768, bscale = 1;
    long naxes[2] = {(long) hdr->ydim, (long) hdr->xdim}; // width x height
    if (!fits_create_file(&fptr, fitsname, &status))
    {
        char buf[13]; // buffer to store strings
        memset(buf, 0x0, sizeof(buf));
        fits_create_img(fptr, bitpix, naxis, naxes, &status);
        for (int i = 0; i < sizeof(hdr->sitename); i++)
            buf[i] = hdr->sitename[i];
        fits_write_key(fptr, TSTRING, "SITENAME", buf, NULL, &status);
        memset(buf, 0x0, sizeof(buf));
        for (int i = 0; i < sizeof(hdr->filename); i++)
            buf[i] = hdr->filename[i];
        fits_write_key(fptr, TSTRING, "FILENAME", buf, NULL, &status);
        memset(buf, 0x0, sizeof(buf));
        for (int i = 0; i < sizeof(hdr->filter); i++)
            buf[i] = hdr->filter[i];
        fits_write_key(fptr, TSTRING, "FILTER", buf, NULL, &status);
        memset(buf, 0x0, sizeof(buf));
        fits_write_key(fptr, TUSHORT, "XDIM", &hdr->xdim, NULL, &status);
        fits_write_key(fptr, TUSHORT, "YDIM", &hdr->ydim, NULL, &status);
        fits_write_key(fptr, TUSHORT, "YEAR", &hdr->year, NULL, &status);
        for (int i = 0; i < sizeof(hdr->month); i++)
            buf[i] = hdr->month[i];
        fits_write_key(fptr, TSTRING, "MONTH", buf, NULL, &status);
        memset(buf, 0x0, sizeof(buf));
        fits_write_key(fptr, TUSHORT, "DAY", &hdr->day, NULL, &status);
        fits_write_key(fptr, TUSHORT, "XBIN", &hdr->xbin, NULL, &status);
        fits_write_key(fptr, TUSHORT, "YBIN", &hdr->ybin, NULL, &status);
        fits_write_key(fptr, TUSHORT, "NUMSU", &hdr->numsu, NULL, &status);
        fits_write_key(fptr, TUSHORT, "LEFT", &hdr->left, NULL, &status);
        fits_write_key(fptr, TUSHORT, "RIGHT", &hdr->right, NULL, &status);
        fits_write_key(fptr, TUSHORT, "TOP", &hdr->top, NULL, &status);
        fits_write_key(fptr, TUSHORT, "BOTTOM", &hdr->bottom, NULL, &status);
        fits_write_key(fptr, TUSHORT, "NUMCO", &hdr->numco, NULL, &status);
        fits_write_key(fptr, TFLOAT, "EXPTIME", &hdr->exposure, NULL, &status);
        fits_write_key(fptr, TFLOAT, "WAITTIME", &hdr->waittime, NULL, &status);
        fits_write_key(fptr, TFLOAT, "TEMPERATURE", &hdr->temperature, NULL, &status);
        long fpixel[] = {1, 1};
        fits_write_pix(fptr, TUSHORT, fpixel, hdr->xdim * hdr->ydim, data, &status);
        fits_close_file(fptr, &status);
    }
    else    
    {
        eprintlf("Error creating fits file");
        return false;
    }
    return true;
}

bool convert_file(char *fname)
{
    bool retval = false;
    // check file name not NULL
    if (fname == NULL)
    {
        eprintlf("File name is NULL");
        goto ret_err;
    }
    // get file size
    ssize_t fsize = get_fsize(fname);
    if (fsize < 0)
        goto ret_err;
    // check if file is at least size of header
    if (fsize < sizeof(hitmis_header))
    {
        eprintlf("File %s has size %ld, which is smaller than %ld bytes.", fname, fsize, sizeof(hitmis_header));
        goto ret_err;
    }
    // open file
    int fd = open(fname, O_RDONLY);
    if (fd < 3)
    {
        eprintlf("Error opening file %s: %s", fname, strerror(errno));
        goto ret_err;
    }
    hitmis_header hdr[1];
    memset(hdr, 0x0, sizeof(hitmis_header));
    // read in header
    ssize_t rd = read(fd, hdr->bytes, sizeof(hitmis_header));
    if (rd != sizeof(hitmis_header))
    {
        eprintlf("Could not read %lu bytes for header, read %ld bytes", sizeof(hitmis_header), rd);
        goto clean;
    }
    // calculate expected size
    ssize_t img_size = hdr->xdim * hdr->ydim * sizeof(uint16_t);
    ssize_t fsize_expect = img_size + sizeof(hitmis_header);
    if (fsize_expect != fsize)
    {
        eprintlf("Expected file size %ld, file size %ld", fsize_expect, fsize);
        print_header(hdr);
        goto clean;
    }
    // print header
    print_header(hdr);
    // load rest of data
    uint16_t *data = (uint16_t *) malloc(img_size);
    if (data == NULL)
    {
        eprintlf("Could not allocate memory to read in data");
        goto clean;
    }
    rd = read(fd, data, img_size);
    if (rd != img_size)
    {
        eprintlf("Error reading %ld bytes, read %ld bytes: %s", img_size, rd, strerror(errno));
        goto clean_mem;
    }
    if (savefit("temp", hdr, data) == false)
    {
        eprintf("Error converting file to FITS");
        goto clean_mem;
    }
    retval = true;
clean_mem:
    free(data);
clean:
    close(fd);
ret_err:
    return retval;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage:\n%s <HITMIS File Name>\n\n", argv[0]);
        return 0;
    }
    bool ret = convert_file(argv[1]);
    printf("Res: %d\n", ret);
    return (!ret);
}