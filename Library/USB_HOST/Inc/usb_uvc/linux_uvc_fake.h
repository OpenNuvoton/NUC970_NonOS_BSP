#ifndef  _LINUX_UVC_FAKE_H_
#define  _LINUX_UVC_FAKE_H_

/// @cond HIDDEN_SYMBOLS

#ifndef NUVOTON_PORTED
#define NUVOTON_PORTED
#endif


#define UVC_ERROR			sysprintf
#define UVC_WARN			sysprintf
#define UVC_DEBUG			sysprintf
#define UVC_INFO(...)
#define UVC_VDEBUG(...)


#define QUEUE_IMAGE_MAX				4
#define YUYV_MAX_SIZE				(640*480*2)
#define MJPEG_MAX_SIZE				(128*1024)


/* include/linux/time.h */
#define CLOCK_REALTIME				0
#define CLOCK_MONOTONIC				1
#define CLOCK_PROCESS_CPUTIME_ID	2
#define CLOCK_THREAD_CPUTIME_ID		3
#define CLOCK_MONOTONIC_RAW			4
#define CLOCK_REALTIME_COARSE		5
#define CLOCK_MONOTONIC_COARSE		6
#define CLOCK_BOOTTIME				7
#define CLOCK_REALTIME_ALARM		8
#define CLOCK_BOOTTIME_ALARM		9

struct v4l2_device;


/* include/media/media-entity.h */
struct media_pad {
	//struct media_entity *entity;    /* Entity this pad belongs to */
	u16 index;                      /* Pad index in the entity pads array */
	unsigned long flags;            /* Pad flags (MEDIA_PAD_FL_*) */
};


/* include/media/v4l2-dev.h */
#define VFL_TYPE_GRABBER	0
#define VFL_TYPE_VBI		1
#define VFL_TYPE_RADIO		2
#define VFL_TYPE_SUBDEV		3
#define VFL_TYPE_MAX		4


/* include/media/v4l2-dev.h */
struct video_device
{
#if defined(CONFIG_MEDIA_CONTROLLER)
	struct media_entity entity;
#endif
	/* device ops */
	//const struct v4l2_file_operations *fops;

	/* sysfs */
	struct device dev;		/* v4l device */
	//struct cdev *cdev;		/* character device */

	/* Set either parent or v4l2_dev if your driver uses v4l2_device */
	struct device *parent;		/* device parent */
	struct v4l2_device *v4l2_dev;	/* v4l2_device parent */

	/* Control handler associated with this device node. May be NULL. */
	//struct v4l2_ctrl_handler *ctrl_handler;

	/* Priority state. If NULL, then v4l2_dev->prio will be used. */
	//struct v4l2_prio_state *prio;

	/* device info */
	char name[32];
	int vfl_type;
	/* 'minor' is set to -1 if the registration failed */
	int minor;
	u16 num;
	/* use bitops to set/clear/test flags */
	unsigned long flags;
	/* attribute to differentiate multiple indices on one physical device */
	int index;

	/* V4L2 file handles */
	//spinlock_t		fh_lock; /* Lock for all v4l2_fhs */
	//struct list_head	fh_list; /* List of struct v4l2_fh */

	int debug;			/* Activates debug level*/

	/* Video standard vars */
	//v4l2_std_id tvnorms;		/* Supported tv norms */
	//v4l2_std_id current_norm;	/* Current tvnorm */

	/* callbacks */
	//void (*release)(struct video_device *vdev);

	/* ioctl callbacks */
	//const struct v4l2_ioctl_ops *ioctl_ops;

	/* serialization lock */
	//struct mutex *lock;
};


/* include/media/v4l2-subdev.h */
struct v4l2_subdev {
#if defined(CONFIG_MEDIA_CONTROLLER)
	struct media_entity entity;
#endif
	struct list_head list;
	//struct module *owner;
	u32 flags;
	struct v4l2_device *v4l2_dev;
	//const struct v4l2_subdev_ops *ops;
	/* Never call these internal ops from within a driver! */
	//const struct v4l2_subdev_internal_ops *internal_ops;
	/* The control handler of this subdev. May be NULL. */
	//struct v4l2_ctrl_handler *ctrl_handler;
	/* name must be unique */
	char name[64];
	/* can be used to group similar subdevs, value is driver-specific */
	//u32 grp_id;
	/* pointer to private data */
	void *dev_priv;
	void *host_priv;
	/* subdev device node */
	struct video_device *devnode;
};


/* include/media/v4l2-device.h */
struct v4l2_device {
	/* dev->driver_data points to this struct.
	   Note: dev might be NULL if there is no parent device
	   as is the case with e.g. ISA devices. */
	struct device *dev;
#if defined(CONFIG_MEDIA_CONTROLLER)
	struct media_device *mdev;
#endif
	/* used to keep track of the registered subdevs */
	//struct list_head subdevs;
	/* lock this struct; can be used by the driver as well if this
	   struct is embedded into a larger struct. */
	//spinlock_t lock;
	/* unique device name, by default the driver name + bus ID */
	//char name[V4L2_DEVICE_NAME_SIZE];
	/* notify callback called by some sub-devices. */
	//void (*notify)(struct v4l2_subdev *sd,
	//		unsigned int notification, void *arg);
	/* The control handler. May be NULL. */
	//struct v4l2_ctrl_handler *ctrl_handler;
	/* Device's priority state */
	//struct v4l2_prio_state prio;
	/* BKL replacement mutex. Temporary solution only. */
	//struct mutex ioctl_lock;
	/* Keep track of the references to this struct. */
	//struct kref ref;
	/* Release function that is called when the ref count goes to 0. */
	//void (*release)(struct v4l2_device *v4l2_dev);
};


/* include/media/v4l2-dev.h */
static __inline void *video_get_drvdata(struct video_device *vdev)
{
	return dev_get_drvdata(&vdev->dev);
}


/* include/media/v4l2-dev.h */
static __inline void video_set_drvdata(struct video_device *vdev, void *data)
{
	dev_set_drvdata(&vdev->dev, data);
}


/* include/media/v4l2-dev.h */
int __video_register_device(struct video_device *vdev, int type, int nr, int warn_if_nr_in_use);
static __inline int video_register_device(struct video_device *vdev,
		int type, int nr)
{
	return __video_register_device(vdev, type, nr, 1);
}


#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
//#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (unsigned int)(a) - 1)
#define ALIGN(x, a)		__ALIGN_KERNEL((x), (a))


void *kmemdup(const void *src, size_t len, gfp_t gfp);


#include "media.h"
#include "video.h"
#include "videodev2.h"
#include "uvcvideo.h"


extern struct uvc_device  *g_uvc_vdev;
extern struct uvc_streaming  *g_cur_stream;

struct video_device *video_device_alloc(void);
void video_unregister_device(struct video_device *vdev);
void video_device_release(struct video_device *vdev);
int v4l2_device_register(struct device *dev, struct v4l2_device *v4l2_dev);
void v4l2_device_unregister(struct v4l2_device *v4l2_dev);

void  uvcDumpBufferHex(UINT8 *pucBuff, INT nSize);


/// @endcond HIDDEN_SYMBOLS

#endif  /* _LINUX_UVC_FAKE_H_ */


