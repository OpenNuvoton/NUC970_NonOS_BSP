#ifndef  _PLATFORM_H_
#define  _PLATFORM_H_

/// @cond HIDDEN_SYMBOLS


#ifndef NUVOTON_PORTED
#define NUVOTON_PORTED
#endif


#define DISABLE_USB_INT()       sysDisableInterrupt(EHCI_IRQn); \
								sysDisableInterrupt(OHCI_IRQn)
#define ENABLE_USB_INT()		sysEnableInterrupt(EHCI_IRQn); \
								sysEnableInterrupt(OHCI_IRQn)


#define IO_BASE         		0xB0000000
#define NON_CACHE_MASK			0x80000000
#define ECHI_BASE_ADDR			0xB0005000

#define OHCI_BASE_ADDR			0xB0007000

#define readl(addr)          	(*(volatile unsigned long *)(addr))
#define writel(x,addr)       	((*(volatile unsigned long *)(addr)) = (unsigned long)x)

#define USB_SWAP16(x)    		(((x>>8)&0xff)|((x&0xff)<<8))
#define USB_SWAP32(x)    		(((x>>24)&0xff)|((x>>8)&0xff00)|((x&0xff00)<<8)|((x&0xff)<<24))

#define USB_PUT32(x,p)			p[0] = x & 0xff;  			\
								p[1] = (x >> 8) & 0xff;		\
								p[2] = (x >> 16) & 0xff;	\
								p[3] = (x >> 24) & 0xff;

#define DIV_ROUND_UP(n,d) 		(((n) + (d) - 1) / (d))

#define max(a,b)				( ((a) > (b)) ? (a) : (b) )
#define min(a,b)				( ((a) > (b)) ? (b) : (a) )

enum {
		false   = 0,
		true    = 1
};

typedef unsigned char  		__u8;
typedef unsigned short    	__u16;
typedef unsigned int      	__u32;
typedef char				__s8;
typedef short				__s16;
typedef int      			__s32;
typedef unsigned short    	__le16;
typedef unsigned int      	__le32;
typedef unsigned char  		uint8_t;
typedef unsigned short    	uint16_t;
typedef unsigned int      	uint32_t;
typedef unsigned char  		u8;
typedef unsigned short    	u16;
typedef unsigned int      	u32;
typedef unsigned long long	u64;
typedef char				s8;
typedef short				s16;
typedef int					s32;
typedef long long			s64;
typedef long long			__s64;
typedef unsigned long long	__u64;
typedef char				bool;
typedef unsigned int		dma_addr_t;
typedef int					ssize_t;
typedef int					atomic_t;
typedef int					gfp_t;
typedef int					spinlock_t;
typedef void				irqreturn_t;
typedef unsigned int		mode_t;

#define __le16_to_cpu(x)	(x)
#define __le32_to_cpu(x)	(x)
#define le16_to_cpu(x)		(x)
#define le32_to_cpu(x)		(x)
#define le16_to_cpus(x)		
#define le32_to_cpus(x)		
#define le32_to_cpup(x)		(*(__u32 *)(x))
#define cpu_to_le16(x)		(x)
#define cpu_to_le32(x)		(x)

static __inline __u16 __le16_to_cpup(const __le16 *p)
{
	return (__u16)*p;
}

static __inline __u16 le16_to_cpup(const __le16 *p)
{
	return (__u16)*p;
}


#define inline				__inline
#define __force			
#define __user
#define pm_message_t		int
#define __iomem
#define __init
#define __exit
#define __maybe_unused
#define irqreturn_t			void

#define BUG()				{ sysprintf("BUG!! %s\n", __func__);  while (1) {} }
#define BUG_ON(x)			{ if (x) { sysprintf("BUG!! %s\n", __func__); while (1); } }
#define WARN()				sysprintf("WARN!! %s\n", __func__);
#define WARN_ON(x)			if (x) sysprintf("WARN!! %s\n", __func__);


#define likely(x)			(x)
#define unlikely(x)			(x)

#define atomic_set(x,v)		(*(x) = v)
#define atomic_read(v)		(*(v))
#define atomic_inc(v)		((*(v))++)
#define atomic_dec(v)		((*(v))--)

#define rmb()
#define wmb()

#define strlcpy				strncpy
#define strlcat				strncat

#define kmalloc(x,f)		USB_malloc(x,4)
#define kfree				USB_free

#define dma_pool_free(p,m,d)	USB_free(m)

#define spin_lock(x)			DISABLE_USB_INT()
#define spin_unlock(x)			ENABLE_USB_INT()
#define mutex_init(x)
#define mutex_lock(x)			DISABLE_USB_INT()
#define mutex_unlock(x)			ENABLE_USB_INT()

#define spin_lock_irq(x)		DISABLE_USB_INT()
#define spin_unlock_irq(x)		ENABLE_USB_INT()
#define spin_lock_irqsave(x,y)		DISABLE_USB_INT()
#define spin_unlock_irqrestore(x,y)	ENABLE_USB_INT()

#define jiffies					(sysGetTicks(TIMER0)*1000)
#define msecs_to_jiffies(x)		(((x/10)+1)*10)
#define time_after_eq(a,b)		((a) >= (b))

#define might_sleep()

#define MAX_SCHEDULE_TIMEOUT		0x7FFFFFFF

#define dev_name(x)				(dev->init_name)


static __inline __u16 get_unaligned_le16(const void *_ptr)
{
	const __u8 *ptr = _ptr;
	return ptr[0] | (ptr[1] << 8);
}

static __inline __u32 get_unaligned_le32(const void *_ptr)
{
	const __u8 *ptr = _ptr;
	return ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
}

static __inline void put_unaligned_le32(__u32 data, void *_ptr)
{
	__u8 *ptr = _ptr;
	ptr[0] = data & 0xff;
	ptr[1] = (data >> 8) & 0xff;
	ptr[2] = (data >> 16) & 0xff;
	ptr[3] = (data >> 24) & 0xff;
}


/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

/// @endcond HIDDEN_SYMBOLS

#endif  /* _PLATFORM_H_ */


