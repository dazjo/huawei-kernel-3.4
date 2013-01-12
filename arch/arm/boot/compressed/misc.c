/*
 * misc.c
 * 
 * This is a collection of several routines from gzip-1.0.3 
 * adapted for Linux.
 *
 * malloc by Hannu Savolainen 1993 and Matthias Urlichs 1994
 *
 * Modified for ARM Linux by Russell King
 *
 * Nicolas Pitre <nico@visuaide.com>  1999/04/14 :
 *  For this code to run directly from Flash, all constant variables must
 *  be marked with 'const' and all other variables initialized at run-time 
 *  only.  This way all non constant variables will end up in the bss segment,
 *  which should point to addresses in RAM and cleared to 0 on start.
 *  This allows for a much quicker boot time.
 */

unsigned int __machine_arch_type;

#include <linux/compiler.h>	/* for inline */
#include <linux/types.h>
#include <linux/linkage.h>

#ifdef CONFIG_SRECORDER_MSM
#ifdef CONFIG_SRECORDER_POWERCOLLAPS
/*----local macroes------------------------------------------------------------------*/

#define SRECORDER_RESERVED_MEM_BANK_BASE (0x20000000) /* 512MB */
#define SRECORDER_RESERVED_MEM_BANK_SIZE (0x10000000) /* 256MB */
#ifndef CONFIG_SRECORDER_RESERVED_MEM_SIZE
#define SRECORDER_RESERVED_MEM_SIZE (0x80000) /* 512KB */
#endif
#define SRECORDER_MAGIC_NUM (0x20122102)
#define INVALID_SRECORDER_MAGIC_NUM (0xffffffff)
#define CRASH_REASON_POWER_COLLAPSE "Crash reason: powercollapse "
#define CRASH_DEFAULT_TIME_POWER_COLLAPSE "Crash time: Unknown\n"
#define CRASH_REASON_KEYWORD "===============crash reason time===============\n"
#define DMESG_KEYWORD "===============dmesg===============\n"
#define ABNORMAL_RESET (1)
#define NORMAL_RESET (0)
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define LOG_BUF_MASK(log_buf_len) (log_buf_len - 1)


/*----local prototypes----------------------------------------------------------------*/

/* SRecorder reserved mem info header */
typedef struct __srecorder_reserved_mem_header_t
{
    unsigned long version; /* the version of the SRecorder */
    unsigned long magic_num; /* the default magic number is 0x20122102 */
    unsigned long data_length; /* log length except this header */

    /*############################################################
   # allocate 6 members for abnormal reset begin
   ############################################################*/
    unsigned long reset_flag; /* system reset_flag, 1 - abnormal reset, 0 - normal reset */
    unsigned long log_buf; /* phys addr of __log_buf */
    unsigned long log_end; /* phys addr of log_end */
    unsigned long log_buf_len; /* phys addr of log_buf_len */
    unsigned long reserved_mem_size; /* SRecorder reserved mem max length */
    unsigned long crc32; /* crc32 check value */
    /*############################################################
   # allocate 6 members for abnormal reset end
   ############################################################*/
    
    char reserved[24];
    
    /*the region following this struct is assigned for the data region*/
}srecorder_reserved_mem_header_t, *psrecorder_reserved_mem_header_t;

/* log type */
typedef enum
{
    CRASH_REASON_TIME = 0, 
    SYS_INFO,
    DMESG,
    ALL_CPU_STACK,    
    ALL_PS_INFO,    
    CURRENT_PS_BACKTRACE,
    SLABINFO,
    MODEM_ERR,
    MODEM_ERR_F3,
    TYPE_MAX,
} srecorder_info_type_e;

/* header for every type of log */
typedef struct __info_header
{
    srecorder_info_type_e type;
    unsigned long crc32;
    unsigned long data_len;
} srecorder_info_header_t, *psrecorder_info_header_t;


/*----local variables------------------------------------------------------------------*/

/* CRC32 table */
static const unsigned long s_crc32tab[256] =
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};


/*----local function prototypes---------------------------------------------------------*/

static unsigned long get_crc32(unsigned char const *pbuf, unsigned long data_len);
static inline bool is_machine_abnormal_reset(unsigned long *psrecorder_reserved_mem_start_addr);
static inline int strcpy_valid_char(char *pdst, char *psrc, int bytes_to_write);
static inline void copy_data(char *pdst, char *psrc, int bytes_to_copy);
static inline int str_len(char *psrc);
static inline void write_info_header(unsigned long header_addr, 
    srecorder_info_type_e type, unsigned long data_len, char *keyword);
static void dump_abnormal_reset_log(unsigned long srecorder_reserved_mem_start_addr);
#endif /* CONFIG_SRECORDER_POWERCOLLAPS */
#endif /* CONFIG_SRECORDER_MSM */

static void putstr(const char *ptr);
extern void error(char *x);

#include <mach/uncompress.h>

#ifdef CONFIG_DEBUG_ICEDCC

#if defined(CONFIG_CPU_V6) || defined(CONFIG_CPU_V6K) || defined(CONFIG_CPU_V7)

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c0, c1, 0" : "=r" (status));
	} while (status & (1 << 29));

	asm("mcr p14, 0, %0, c0, c5, 0" : : "r" (ch));
}


#elif defined(CONFIG_CPU_XSCALE)

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c14, c0, 0" : "=r" (status));
	} while (status & (1 << 28));

	asm("mcr p14, 0, %0, c8, c0, 0" : : "r" (ch));
}

#else

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c0, c0, 0" : "=r" (status));
	} while (status & 2);

	asm("mcr p14, 0, %0, c1, c0, 0" : : "r" (ch));
}

#endif

#define putc(ch)	icedcc_putc(ch)
#endif

static void putstr(const char *ptr)
{
	char c;

	while ((c = *ptr++) != '\0') {
		if (c == '\n')
			putc('\r');
		putc(c);
	}

	flush();
}

/*
 * gzip declarations
 */
extern char input_data[];
extern char input_data_end[];

unsigned char *output_data;

unsigned long free_mem_ptr;
unsigned long free_mem_end_ptr;

#ifndef arch_error
#define arch_error(x)
#endif

void error(char *x)
{
	arch_error(x);

	putstr("\n\n");
	putstr(x);
	putstr("\n\n -- System halted");

	while(1);	/* Halt */
}

asmlinkage void __div0(void)
{
	error("Attempting division by 0!");
}

extern int do_decompress(u8 *input, int len, u8 *output, void (*error)(char *x));


#ifdef CONFIG_SRECORDER_MSM
#ifdef CONFIG_SRECORDER_POWERCOLLAPS
/**
    @function: static unsigned long get_crc32(unsigned char const *pbuf, unsigned long data_len)
    @brief: crc32 check

    @param: pbuf data buffer to be checked
    @param: data_len data length
    
    @return: CRC32 check value
    
    @note: 
*/
static unsigned long get_crc32(unsigned char const *pbuf, unsigned long data_len)
{     
    unsigned int crc32;
    unsigned long i = 0;

    if (NULL == pbuf)
    {
        return 0xFFFFFFFF;
    }
    
    /* 开始计算CRC32校验值 */
    crc32 = 0xFFFFFFFF;     
    for (i = 0; i < data_len; i++) 
    {
        crc32 = (crc32 >> 8) ^ s_crc32tab[(crc32 & 0xFF) ^ pbuf[i]];     
    }  

    return crc32 ^= 0xFFFFFFFF; 
}   


/**
    @function: static inline int strcpy_valid_char(char *pdst, char *psrc, int bytes_to_write)
    @brief: write bytes_to_write bytes data form psrc to pdst

    @param: pdst
    @param: psrc 
    @param: bytes_to_write 
    
    @return: the bytes have been written to the pdst
    
    @note: 
*/
static inline int strcpy_valid_char(char *pdst, char *psrc, int bytes_to_write)
{
    int i = 0;
    int j = 0;
    char c = 0;
    
    for (i = 0; i < bytes_to_write; i++)
    {
        c = *(psrc + i);
        
        if (c > 127 || 0 == c)
        {
            continue;
        }

        *(pdst + j) = c;
        j++;
    }

    return j;
}


/**
    @function: static inline void copy_data(char *pdst, char *psrc, int bytes_to_copy)
    @brief: copy data form psrc to pdst

    @param: pdst
    @param: psrc 
    @param: bytes_to_copy 
    
    @return: none
    
    @note: 
*/
static inline void copy_data(char *pdst, char *psrc, int bytes_to_copy)
{
    while (bytes_to_copy-- > 0)
    {
        *pdst++ = *psrc++;
    }
}


/**
    @function: static inline int str_len(char *psrc)
    @brief: calculate the string's length

    @param: psrc
    
    @return: length of the string
    
    @note: 
*/
static inline int str_len(char *psrc)
{
    char *ptemp = psrc;

    for (ptemp = psrc; '\0' != *psrc; psrc++)
    {
        ;
    }

    return psrc - ptemp;
}


/**
    @function: static inline void write_info_header(unsigned long header_addr, 
        srecorder_info_type_e type, unsigned long data_len, char *keyword)
    @brief: header including check 12 bytes check header and the keyword of for every type of log

    @param: header_addr header start address
    @param: type log type
    @param: data_len lengh of the valid data including keyword and the content for every type of log
    @param: keyword log's description 
    
    @return: none
    
    @note: 
*/
static inline void write_info_header(unsigned long header_addr, 
    srecorder_info_type_e type, unsigned long data_len, char *keyword)
{
    unsigned long buf[2] = {0, 0};
    srecorder_info_header_t info_header;
    
    buf[0] = (unsigned long)type;
    buf[1] = (unsigned long)data_len;
    info_header.type = type;
    info_header.data_len = data_len;
    info_header.crc32 = get_crc32((unsigned char *)buf, sizeof(buf));
    
    copy_data((char *)header_addr, (char *)&info_header, sizeof(srecorder_info_header_t));
    strcpy_valid_char((char *)(header_addr + sizeof(srecorder_info_header_t)), keyword, str_len(keyword));
}


/**
    @function: static inline void dump_abnormal_reset_log(unsigned long srecorder_reserved_mem_start_addr)
    @brief: dump log to the SRecorder's reserved memory

    @param: srecorder_reserved_mem_start_addr physical start address of the SRecorder's reserved memory zone.

    @return: none
    
    @note: 
*/
static inline void dump_abnormal_reset_log(unsigned long srecorder_reserved_mem_start_addr)
{
    psrecorder_reserved_mem_header_t pheader = (psrecorder_reserved_mem_header_t)srecorder_reserved_mem_start_addr;
    int mem_header_size = sizeof(srecorder_reserved_mem_header_t);
    int log_header_size = sizeof(srecorder_info_header_t);
    int bytes_to_dumped = 0;
    int data_len = 0;
    int keyword_len = 0;
    unsigned log_end = *(unsigned *)pheader->log_end;
    unsigned log_buf_len = *(unsigned *)pheader->log_buf_len;
    unsigned long log_header_start_addr = 0x0;
    char *log_buf = (char *)pheader->log_buf; /* NOTE: it equals to __log_buf's address */
    char *pstart = NULL;
    
    /* 1. dump crash reason */
    log_header_start_addr = srecorder_reserved_mem_start_addr + mem_header_size;
    keyword_len = str_len(CRASH_REASON_KEYWORD);
    pstart = (char *)(log_header_start_addr + log_header_size + keyword_len);
    data_len = strcpy_valid_char(pstart, CRASH_REASON_POWER_COLLAPSE, str_len(CRASH_REASON_POWER_COLLAPSE));
    data_len += strcpy_valid_char(pstart + data_len, CRASH_DEFAULT_TIME_POWER_COLLAPSE, str_len(CRASH_DEFAULT_TIME_POWER_COLLAPSE));
    write_info_header(log_header_start_addr, CRASH_REASON_TIME, keyword_len + data_len, CRASH_REASON_KEYWORD);
    log_header_start_addr += (log_header_size + keyword_len + data_len);
    pheader->data_length += (log_header_size + keyword_len + data_len);
    
    /* 2.dump dmesg */
    data_len = 0;
    log_end &= LOG_BUF_MASK(log_buf_len);
    keyword_len = str_len(DMESG_KEYWORD);
    pstart = (char *)(log_header_start_addr + log_header_size + keyword_len);
    bytes_to_dumped = MIN((srecorder_reserved_mem_start_addr + pheader->reserved_mem_size 
        - log_header_start_addr - log_header_size - keyword_len), log_buf_len); 
    if (log_end >= bytes_to_dumped)
    {
        data_len = strcpy_valid_char(pstart, log_buf + (log_end - bytes_to_dumped), bytes_to_dumped);
    }
    else
    {
        data_len = strcpy_valid_char(pstart, log_buf + (log_buf_len - (bytes_to_dumped - log_end)), (bytes_to_dumped - log_end));
        data_len += strcpy_valid_char(pstart + data_len, log_buf, log_end);
    }
    write_info_header(log_header_start_addr, DMESG, keyword_len + data_len, DMESG_KEYWORD);
    pheader->data_length += (log_header_size + keyword_len + data_len);

    /* 3. update the reset_flasg and magic_num of the mem info header */
    pheader->reset_flag = NORMAL_RESET;
    pheader->magic_num = SRECORDER_MAGIC_NUM;
}


/**
    @function: static inline bool is_machine_abnormal_reset(unsigned long *psrecorder_reserved_mem_start_addr)
    @brief: check if the system has been abnaormally reset, If the anwser is "YES", and return the physical start 
            address of the SRecorder's reserved memory zone.

    @param: psrecorder_reserved_mem_start_addr to save the start address of SRecorder's reserved memory zone.
    
    @return: true - yes, it has been reset abnormally, false - no
    
    @note: 
*/
static inline bool is_machine_abnormal_reset(unsigned long *psrecorder_reserved_mem_start_addr)
{
    psrecorder_reserved_mem_header_t pheader = (psrecorder_reserved_mem_header_t)SRECORDER_RESERVED_MEM_BANK_BASE;
    unsigned long data_len = sizeof(pheader->version) 
        + sizeof(pheader->magic_num) 
        + sizeof(pheader->data_length) 
        + sizeof(pheader->reset_flag) 
        + sizeof(pheader->log_buf) 
        + sizeof(pheader->log_end) 
        + sizeof(pheader->log_buf_len) 
        + sizeof(pheader->reserved_mem_size);

    while (((unsigned long)pheader - SRECORDER_RESERVED_MEM_BANK_BASE) < SRECORDER_RESERVED_MEM_BANK_SIZE)
    {
        if (get_crc32((unsigned char *)pheader, data_len) == pheader->crc32 
            && ABNORMAL_RESET == pheader->reset_flag
            && INVALID_SRECORDER_MAGIC_NUM == pheader->magic_num)
        {
            *psrecorder_reserved_mem_start_addr = (unsigned long)pheader;
            return true;
        }
        
#ifdef CONFIG_SRECORDER_RESERVED_MEM_SIZE
        pheader = (psrecorder_reserved_mem_header_t)((unsigned long)pheader + (CONFIG_SRECORDER_RESERVED_MEM_SIZE));
#else
        pheader = (psrecorder_reserved_mem_header_t)((unsigned long)pheader + SRECORDER_RESERVED_MEM_SIZE);
#endif
    }
    
    return false;
}
#endif /* CONFIG_SRECORDER_POWERCOLLAPS */
#endif /* CONFIG_SRECORDER_MSM */


void
decompress_kernel(unsigned long output_start, unsigned long free_mem_ptr_p,
		unsigned long free_mem_ptr_end_p,
		int arch_id)
{
	int ret;
#ifdef CONFIG_SRECORDER_MSM
#ifdef CONFIG_SRECORDER_POWERCOLLAPS
    unsigned long srecorder_reserved_mem_start_addr = 0x0;
#endif /* CONFIG_SRECORDER_POWERCOLLAPS */
#endif /* CONFIG_SRECORDER_MSM */
    
	output_data		= (unsigned char *)output_start;
	free_mem_ptr		= free_mem_ptr_p;
	free_mem_end_ptr	= free_mem_ptr_end_p;
	__machine_arch_type	= arch_id;

#ifdef CONFIG_SRECORDER_MSM
#ifdef CONFIG_SRECORDER_POWERCOLLAPS
    if (is_machine_abnormal_reset(&srecorder_reserved_mem_start_addr))
    {
        /* dump dmesg */
        dump_abnormal_reset_log(srecorder_reserved_mem_start_addr);
    }
#endif /* CONFIG_SRECORDER_POWERCOLLAPS */
#endif /* CONFIG_SRECORDER_MSM */

	arch_decomp_setup();

	putstr("Uncompressing Linux...");
	ret = do_decompress(input_data, input_data_end - input_data,
			    output_data, error);
	if (ret)
		error("decompressor returned an error");
	else
		putstr(" done, booting the kernel.\n");
}
