/*************************************************************************
 * Nuvoton Electronics Corporation confidential
 *
 * Copyright (c) 2012 by Nuvoton Electronics Corporation
 * All rights reserved
 *
 * FILENAME
 *     support.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     UVC driver 
 *
 * HISTORY
 *     2012.04.22       Created
 *
 * REMARK
 *     None
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nuc970.h"
#include "sys.h"
#include "usb.h"
#include "usbh_lib.h"
#include "linux_uvc_fake.h"

void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
	void *p;
	p = malloc(len);
	if (p)
		memcpy(p, src, len);
	return p;
}


//static __inline int video_is_registered(struct video_device *vdev)
//{
//	return test_bit(V4L2_FL_REGISTERED, &vdev->flags);
//}


/* drivers/media/video/v4l2-dev.c */
struct video_device *video_device_alloc(void)
{
	return kzalloc(sizeof(struct video_device), GFP_KERNEL);
}


/* drivers/media/video/v4l2-dev.c */
void video_device_release(struct video_device *vdev)
{
	kfree(vdev);
}


int __video_register_device(struct video_device *vdev, int type, int nr, int warn_if_nr_in_use)
{
	//int i = 0;
	//int ret;
	//int minor_offset = 0;
	//int minor_cnt = VIDEO_NUM_DEVICES;
	//const char *name_base;

	/* A minor value of -1 marks this video device as never
	   having been registered */
	vdev->minor = -1;

	/* the release callback MUST be present */
	//WARN_ON(!vdev->release);
	//if (!vdev->release)
	//	return -EINVAL;

	/* v4l2_fh support */
	//spin_lock_init(&vdev->fh_lock);
	//INIT_LIST_HEAD(&vdev->fh_list);

	/* Part 1: check device type */
	//switch (type) {
	//case VFL_TYPE_GRABBER:
	//	name_base = "video";
	//	break;
	//case VFL_TYPE_VBI:
	//	name_base = "vbi";
	//	break;
	//case VFL_TYPE_RADIO:
	//	name_base = "radio";
	//	break;
	//case VFL_TYPE_SUBDEV:
	//	name_base = "v4l-subdev";
	//	break;
	//default:
	//	UVC_ERROR("%s called with unknown type: %d\n",__func__, type);
	//	return -EINVAL;
	//}

	vdev->vfl_type = type;
	//vdev->cdev = NULL;
	if (vdev->v4l2_dev) 
	{
		if (vdev->v4l2_dev->dev)
			vdev->parent = vdev->v4l2_dev->dev;
		//if (vdev->ctrl_handler == NULL)
		//	vdev->ctrl_handler = vdev->v4l2_dev->ctrl_handler;
		/* If the prio state pointer is NULL, then use the v4l2_device
		   prio state. */
		//if (vdev->prio == NULL)
		//	vdev->prio = &vdev->v4l2_dev->prio;
	}

	/* Part 2: find a free minor, device node number and device index. */
#ifdef CONFIG_VIDEO_FIXED_MINOR_RANGES
	/* Keep the ranges for the first four types for historical
	 * reasons.
	 * Newer devices (not yet in place) should use the range
	 * of 128-191 and just pick the first free minor there
	 * (new style). */
	switch (type) {
	case VFL_TYPE_GRABBER:
		minor_offset = 0;
		minor_cnt = 64;
		break;
	case VFL_TYPE_RADIO:
		minor_offset = 64;
		minor_cnt = 64;
		break;
	case VFL_TYPE_VBI:
		minor_offset = 224;
		minor_cnt = 32;
		break;
	default:
		minor_offset = 128;
		minor_cnt = 64;
		break;
	}
#endif

	/* Pick a device node number */
	mutex_lock(&videodev_lock);
	//nr = devnode_find(vdev, nr == -1 ? 0 : nr, minor_cnt);
	//if (nr == minor_cnt)
	//	nr = devnode_find(vdev, 0, minor_cnt);
	//if (nr == minor_cnt) {
	//	printk(KERN_ERR "could not get a free device node number\n");
	//	mutex_unlock(&videodev_lock);
	//	return -ENFILE;
	//}
#ifdef CONFIG_VIDEO_FIXED_MINOR_RANGES
	/* 1-on-1 mapping of device node number to minor number */
	i = nr;
#else
	/* The device node number and minor numbers are independent, so
	   we just find the first free minor number. */
	//for (i = 0; i < VIDEO_NUM_DEVICES; i++)
	//	if (video_device[i] == NULL)
	//		break;
	//if (i == VIDEO_NUM_DEVICES) {
	//	mutex_unlock(&videodev_lock);
	//	printk(KERN_ERR "could not get a free minor\n");
	//	return -ENFILE;
	//}
#endif
	//vdev->minor = i + minor_offset;
	//vdev->num = nr;
	//devnode_set(vdev);

	/* Should not happen since we thought this minor was free */
	//WARN_ON(video_device[vdev->minor] != NULL);
	//vdev->index = get_index(vdev);
	mutex_unlock(&videodev_lock);

	/* Part 3: Initialize the character device */
	//vdev->cdev = cdev_alloc();
	//if (vdev->cdev == NULL) {
	//	ret = -ENOMEM;
	//	goto cleanup;
	//}
	//vdev->cdev->ops = &v4l2_fops;
	//vdev->cdev->owner = owner;
	//ret = cdev_add(vdev->cdev, MKDEV(VIDEO_MAJOR, vdev->minor), 1);
	//if (ret < 0) {
	//	printk(KERN_ERR "%s: cdev_add failed\n", __func__);
	//	kfree(vdev->cdev);
	//	vdev->cdev = NULL;
	//	goto cleanup;
	//}

	/* Part 4: register the device with sysfs */
	//vdev->dev.class = &video_class;
	//vdev->dev.devt = MKDEV(VIDEO_MAJOR, vdev->minor);
	//if (vdev->parent)
	//	vdev->dev.parent = vdev->parent;
	//dev_set_name(&vdev->dev, "%s%d", name_base, vdev->num);
	//ret = device_register(&vdev->dev);
	//if (ret < 0) {
	//	printk(KERN_ERR "%s: device_register failed\n", __func__);
	//	goto cleanup;
	//}
	/* Register the release callback that will be called when the last
	   reference to the device goes away. */
	//vdev->dev.release = v4l2_device_release;

	//if (nr != -1 && nr != vdev->num && warn_if_nr_in_use)
	//	printk(KERN_WARNING "%s: requested %s%d, got %s\n", __func__,
	//		name_base, nr, video_device_node_name(vdev));

	/* Increase v4l2_device refcount */
	//if (vdev->v4l2_dev)
	//	v4l2_device_get(vdev->v4l2_dev);

#if defined(CONFIG_MEDIA_CONTROLLER)
	/* Part 5: Register the entity. */
	if (vdev->v4l2_dev && vdev->v4l2_dev->mdev &&
	    vdev->vfl_type != VFL_TYPE_SUBDEV) {
		vdev->entity.type = MEDIA_ENT_T_DEVNODE_V4L;
		vdev->entity.name = vdev->name;
		vdev->entity.v4l.major = VIDEO_MAJOR;
		vdev->entity.v4l.minor = vdev->minor;
		ret = media_device_register_entity(vdev->v4l2_dev->mdev,
			&vdev->entity);
		if (ret < 0)
			printk(KERN_WARNING
			       "%s: media_device_register_entity failed\n",
			       __func__);
	}
#endif
	/* Part 6: Activate this minor. The char device can now be used. */
	//set_bit(V4L2_FL_REGISTERED, &vdev->flags);
	mutex_lock(&videodev_lock);
	//video_device[vdev->minor] = vdev;
	mutex_unlock(&videodev_lock);

	return 0;

//cleanup:
//	mutex_lock(&videodev_lock);
	//if (vdev->cdev)
	//	cdev_del(vdev->cdev);
	//devnode_clear(vdev);
//	mutex_unlock(&videodev_lock);
	/* Mark this video device as never having been registered. */
//	vdev->minor = -1;
//	return ret;
}


/* drivers/media/video/v4l2-dev.c */
void video_unregister_device(struct video_device *vdev)
{
	/* Check if vdev was ever registered at all */
	if (!vdev)		// || !video_is_registered(vdev))
		return;

	mutex_lock(&videodev_lock);
	/* This must be in a critical section to prevent a race with v4l2_open.
	 * Once this bit has been cleared video_get may never be called again.
	 */
	//clear_bit(V4L2_FL_REGISTERED, &vdev->flags);
	mutex_unlock(&videodev_lock);
	//device_unregister(&vdev->dev);
}


/* drivers/media/video/v4l2-device.c */
int v4l2_device_register(struct device *dev, struct v4l2_device *v4l2_dev)
{
	if (v4l2_dev == NULL)
		return -EINVAL;

	//INIT_LIST_HEAD(&v4l2_dev->subdevs);
	//spin_lock_init(&v4l2_dev->lock);
	//mutex_init(&v4l2_dev->ioctl_lock);
	//v4l2_prio_init(&v4l2_dev->prio);
	//kref_init(&v4l2_dev->ref);
	//get_device(dev);
	v4l2_dev->dev = dev;
	//if (dev == NULL) {
	//	/* If dev == NULL, then name must be filled in by the caller */
	//	WARN_ON(!v4l2_dev->name[0]);
	//	return 0;
	//}

	/* Set name to driver name + device name if it is empty. */
	//if (!v4l2_dev->name[0])
	//	snprintf(v4l2_dev->name, sizeof(v4l2_dev->name), "%s %s",
	//		dev->driver->name, dev_name(dev));
	if (!dev_get_drvdata(dev))
		dev_set_drvdata(dev, v4l2_dev);
	return 0;
}


/* drivers/media/video/v4l2-device.c */
void v4l2_device_disconnect(struct v4l2_device *v4l2_dev)
{
	if (v4l2_dev->dev == NULL)
		return;

	if (dev_get_drvdata(v4l2_dev->dev) == v4l2_dev)
		dev_set_drvdata(v4l2_dev->dev, NULL);
	//put_device(v4l2_dev->dev);
	v4l2_dev->dev = NULL;
}


/* drivers/media/video/v4l2-device.c */
void v4l2_device_unregister_subdev(struct v4l2_subdev *sd)
{
	//struct v4l2_device *v4l2_dev;

	/* return if it isn't registered */
	if (sd == NULL || sd->v4l2_dev == NULL)
		return;

	//v4l2_dev = sd->v4l2_dev;

	spin_lock(&v4l2_dev->lock);
	list_del(&sd->list);
	spin_unlock(&v4l2_dev->lock);

	//if (sd->internal_ops && sd->internal_ops->unregistered)
	//	sd->internal_ops->unregistered(sd);
	sd->v4l2_dev = NULL;

#if defined(CONFIG_MEDIA_CONTROLLER)
	if (v4l2_dev->mdev)
		media_device_unregister_entity(&sd->entity);
#endif
	video_unregister_device(sd->devnode);
	//module_put(sd->owner);
}


/* drivers/media/video/v4l2-device.c */
void v4l2_device_unregister(struct v4l2_device *v4l2_dev)
{
	//struct v4l2_subdev *sd, *next;

	if (v4l2_dev == NULL)
		return;
	v4l2_device_disconnect(v4l2_dev);

#if 0
	/* Unregister subdevs */
#ifdef NUVOTON_PORTED
	for (sd = list_entry((&v4l2_dev->subdevs)->next, struct v4l2_subdev, list),
		next = list_entry(sd->list.next, typeof(*sd), list);
	     &sd->list != (&v4l2_dev->subdevs);
	     sd = next, next = list_entry(next->list.next, struct v4l2_subdev, list)) {
#else
	list_for_each_entry_safe(sd, next, &v4l2_dev->subdevs, list) {
#endif		
		v4l2_device_unregister_subdev(sd);
	}
#endif	
}



/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/


/**
  * @brief      Check if UVC device is connected or not.
  *
  * @retval     0      UVC device not found.
  * @retval     1      UVC device is connected.
  */
int usbh_uvc_is_device_found()
{
	if (g_uvc_vdev == NULL)
		return 0;
	return 1;
}


/**
  * @brief       Open the UVC device.
  *
  * @param[in]   req_format  Select image format.
  *              - \ref UVC_FORMAT_YUYV         YUYV format image.
  *              - \ref UVC_FORMAT_MJPEG        Motion JPEG image.
  *              - \ref UVC_FORMAT_H264         H.264 video stram.
  *
  * @retval      - \ref UVC_OK                  Success.
  * @retval      - \ref UVC_ERR_MEMORY_OUT      Out of memory.
  * @retval      - \ref UVC_ERR_NO_DEVICE       UVC device not found.
  * @retval      - \ref UVC_ERR_NO_STREAM       Video stream is not found or not enabled. 
  * @retval      - \ref UVC_ERR_IVALID_PARM     Invalid parameter.
  * @retval      - \ref UVC_ERR_NOT_AVAILABLE   Image format or size not supported.
  */
int usbh_uvc_open(int req_format)
{
	struct v4l2_buffer 	buf;
	int   i, ret;

	if (g_uvc_vdev == NULL)
		return UVC_ERR_NO_DEVICE;
		
	if (g_cur_stream == NULL)
	{
		UVC_ERROR("usbh_uvc_set_format() may not be successfully called!\n");
		return UVC_ERR_NO_STREAM;
	}

	uvc_free_buffers(&g_cur_stream->queue);
		
	ret = uvc_alloc_buffers(&g_cur_stream->queue, QUEUE_IMAGE_MAX, 
				(req_format == UVC_FORMAT_YUYV) ? YUYV_MAX_SIZE : MJPEG_MAX_SIZE);
	if (ret < 0)
	{
		UVC_ERROR("uvc_alloc_buffers failed!! %d\n", ret);
		return ret;
	}			
			
				
	for (i = 0; i < QUEUE_IMAGE_MAX; ++i) 
	{
		memset(&buf, 0, sizeof buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = uvc_query_buffer(&g_cur_stream->queue, &buf);
		if (ret < 0)
		{
			UVC_ERROR("uvc_query_buffer failed!\n");
			uvc_free_buffers(&g_cur_stream->queue);
			return ret;
		}
				
		ret = uvc_queue_buffer(&g_cur_stream->queue, &buf);
		if (ret < 0)
		{
			UVC_ERROR("uvc_queue_buffer failed!\n");
			uvc_free_buffers(&g_cur_stream->queue);
			return ret;
		}
	}
		
	return 0;
}


static __u32 uvc_try_frame_interval(struct uvc_frame *frame, __u32 interval)
{
	unsigned int i;

	if (frame->bFrameIntervalType) 
	{
		__u32  best = 0xffffffff, dist;

		for (i = 0; i < frame->bFrameIntervalType; ++i) 
		{
			dist = interval > frame->dwFrameInterval[i]
			     ? interval - frame->dwFrameInterval[i]
			     : frame->dwFrameInterval[i] - interval;

			if (dist > best)
				break;

			best = dist;
		}

		interval = frame->dwFrameInterval[i-1];
	} else {
		const __u32 min = frame->dwFrameInterval[0];
		const __u32 max = frame->dwFrameInterval[1];
		const __u32 step = frame->dwFrameInterval[2];

		interval = min + (interval - min + step/2) / step * step;
		if (interval > max)
			interval = max;
	}

	return interval;
}


static int uvc_v4l2_get_streamparm(struct uvc_streaming *stream, struct v4l2_streamparm *parm)
{
	uint32_t numerator, denominator;

	if (parm->type != stream->type)
		return -EINVAL;

	mutex_lock(&stream->mutex);
	numerator = stream->ctrl.dwFrameInterval;
	mutex_unlock(&stream->mutex);

	denominator = 10000000;
	uvc_simplify_fraction(&numerator, &denominator, 8, 333);

	memset(parm, 0, sizeof *parm);
	parm->type = stream->type;

	if (stream->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		parm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
		parm->parm.capture.capturemode = 0;
		parm->parm.capture.timeperframe.numerator = numerator;
		parm->parm.capture.timeperframe.denominator = denominator;
		parm->parm.capture.extendedmode = 0;
		parm->parm.capture.readbuffers = 0;
	} else {
		parm->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
		parm->parm.output.outputmode = 0;
		parm->parm.output.timeperframe.numerator = numerator;
		parm->parm.output.timeperframe.denominator = denominator;
	}

	return 0;
}


static int uvc_v4l2_set_streamparm(struct uvc_streaming *stream, struct v4l2_streamparm *parm)
{
	struct uvc_streaming_control probe;
	struct v4l2_fract timeperframe;
	uint32_t interval;
	int ret;

	if (parm->type != stream->type)
		return -EINVAL;

	if (parm->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
		timeperframe = parm->parm.capture.timeperframe;
	else
		timeperframe = parm->parm.output.timeperframe;

	interval = uvc_fraction_to_interval(timeperframe.numerator,
		timeperframe.denominator);
	UVC_DEBUG("Setting frame interval to %d (rate: %d).\n", interval, 10000000/interval);

	mutex_lock(&stream->mutex);

	if (uvc_queue_streaming(&stream->queue)) {
		mutex_unlock(&stream->mutex);
		return -EBUSY;
	}

	memcpy(&probe, &stream->ctrl, sizeof probe);
	probe.dwFrameInterval = uvc_try_frame_interval(stream->cur_frame, interval);
	
	UVC_DEBUG(" try frame interval result %d (rate: %d).\n", probe.dwFrameInterval, 10000000/probe.dwFrameInterval);

	/* Probe the device with the new settings. */
	ret = uvc_probe_video(stream, &probe);
	if (ret < 0) {
		mutex_unlock(&stream->mutex);
		return ret;
	}

	memcpy(&stream->ctrl, &probe, sizeof probe);
	mutex_unlock(&stream->mutex);

	/* Return the actual frame period. */
	timeperframe.numerator = probe.dwFrameInterval;
	timeperframe.denominator = 10000000;
	uvc_simplify_fraction(&timeperframe.numerator,
		&timeperframe.denominator, 8, 333);

	if (parm->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
		parm->parm.capture.timeperframe = timeperframe;
	else
		parm->parm.output.timeperframe = timeperframe;

	return 0;
}


static int uvc_v4l2_try_format(struct uvc_streaming *stream,
	struct v4l2_format *fmt, struct uvc_streaming_control *probe,
	struct uvc_format **uvc_format, struct uvc_frame **uvc_frame)
{
	struct uvc_format *format = NULL;
	struct uvc_frame *frame = NULL;
	__u16 rw, rh;
	unsigned int d, maxd;
	unsigned int i;
	__u32 interval;
	int ret = 0;
	__u8 *fcc;

	if (fmt->type != stream->type)
		return -EINVAL;

	fcc = (__u8 *)&fmt->fmt.pix.pixelformat;
	UVC_DEBUG("Trying format 0x%08x (%c%c%c%c): %ux%u.\n",
			fmt->fmt.pix.pixelformat,
			fcc[0], fcc[1], fcc[2], fcc[3],
			fmt->fmt.pix.width, fmt->fmt.pix.height);

	/* Check if the hardware supports the requested format. */
	for (i = 0; i < stream->nformats; ++i) {
		format = &stream->format[i];
		if (format->fcc == fmt->fmt.pix.pixelformat)
			break;
	}

	if (format == NULL || format->fcc != fmt->fmt.pix.pixelformat) {
		UVC_ERROR("Unsupported format 0x%08x.\n", fmt->fmt.pix.pixelformat);
		return UVC_ERR_NOT_AVAILABLE;
	}

	/* Find the closest image size. The distance between image sizes is
	 * the size in pixels of the non-overlapping regions between the
	 * requested size and the frame-specified size.
	 */
	rw = fmt->fmt.pix.width;
	rh = fmt->fmt.pix.height;
	maxd = (unsigned int)-1;

	for (i = 0; i < format->nframes; ++i) {
		__u16 w = format->frame[i].wWidth;
		__u16 h = format->frame[i].wHeight;

		d = min(w, rw) * min(h, rh);
		d = w*h + rw*rh - 2*d;
		if (d < maxd) {
			maxd = d;
			frame = &format->frame[i];
		}

		if (maxd == 0)
			break;
	}

	if (frame == NULL) {
		UVC_ERROR("Unsupported size %ux%u.\n",
				fmt->fmt.pix.width, fmt->fmt.pix.height);
		return UVC_ERR_NOT_AVAILABLE;
	}

	/* Use the default frame interval. */
	interval = frame->dwDefaultFrameInterval;
	UVC_DEBUG("Using default frame interval %d.%d us "
		"(%d.%d fps).\n", interval/10, interval%10, 10000000/interval,
		(100000000/interval)%10);

	/* Set the format index, frame index and frame interval. */
	memset(probe, 0, sizeof *probe);
	probe->bmHint = 1;	/* dwFrameInterval */
	probe->bFormatIndex = format->index;
	probe->bFrameIndex = frame->bFrameIndex;
	probe->dwFrameInterval = uvc_try_frame_interval(frame, interval);
	/* Some webcams stall the probe control set request when the
	 * dwMaxVideoFrameSize field is set to zero. The UVC specification
	 * clearly states that the field is read-only from the host, so this
	 * is a webcam bug. Set dwMaxVideoFrameSize to the value reported by
	 * the webcam to work around the problem.
	 *
	 * The workaround could probably be enabled for all webcams, so the
	 * quirk can be removed if needed. It's currently useful to detect
	 * webcam bugs and fix them before they hit the market (providing
	 * developers test their webcams with the Linux driver as well as with
	 * the Windows driver).
	 */
	mutex_lock(&stream->mutex);
	if (stream->dev->quirks & UVC_QUIRK_PROBE_EXTRAFIELDS)
		probe->dwMaxVideoFrameSize = stream->ctrl.dwMaxVideoFrameSize;

	/* Probe the device. */
	ret = uvc_probe_video(stream, probe);
	mutex_unlock(&stream->mutex);
	if (ret < 0)
		goto done;

	fmt->fmt.pix.width = frame->wWidth;
	fmt->fmt.pix.height = frame->wHeight;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = format->bpp * frame->wWidth / 8;
	fmt->fmt.pix.sizeimage = probe->dwMaxVideoFrameSize;
	fmt->fmt.pix.colorspace = (enum v4l2_colorspace)format->colorspace;
	fmt->fmt.pix.priv = 0;

	if (uvc_format != NULL)
		*uvc_format = format;
	if (uvc_frame != NULL)
		*uvc_frame = frame;

done:
	return ret;
}


/**
  * @brief       Select UVC video stream image format.
  *
  * @param[in]   width   Image width.
  * @param[in]   height  Image height.
  * @param[in]   req_format  Image format.
  *              - \ref UVC_FORMAT_YUYV         YUYV format image.
  *              - \ref UVC_FORMAT_MJPEG        Motion JPEG image.
  *              - \ref UVC_FORMAT_H264         H.264 video stram.
  *
  * @retval      - \ref UVC_OK                  Success.
  * @retval      - \ref UVC_ERR_IVALID_PARM     Invalid parameter.
  * @retval      - \ref UVC_ERR_NOT_AVAILABLE   Image format or size not supported.
  */
int usbh_uvc_set_format(int width, int height, int req_format)
{
	struct v4l2_format 	fmt;
	__u32				pixelformat;
	struct uvc_streaming_control probe;
	struct uvc_format 	*format;
	struct uvc_frame 	*frame;
	int ret;

	if (req_format == UVC_FORMAT_YUYV)
		pixelformat = V4L2_PIX_FMT_YUYV;
	else if (req_format == UVC_FORMAT_MJPEG)
		pixelformat = V4L2_PIX_FMT_MJPEG;
	else
	{
		UVC_ERROR("Unsupported video format!\n");
		return UVC_ERR_IVALID_PARM;
	}
	
	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.pixelformat = pixelformat;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;

	if (fmt.type != g_cur_stream->type)
		return UVC_ERR_NOT_AVAILABLE;

	ret = uvc_v4l2_try_format(g_cur_stream, &fmt, &probe, &format, &frame);
	if (ret < 0)
		return ret;

	mutex_lock(&g_cur_stream->mutex);

	//if (uvc_queue_allocated(&g_cur_stream->queue)) {
	//	ret = -EBUSY;
	//	goto done;
	//}

	memcpy(&g_cur_stream->ctrl, &probe, sizeof probe);
	g_cur_stream->cur_format = format;
	g_cur_stream->cur_frame = frame;

//done:
	mutex_unlock(&g_cur_stream->mutex);
	return ret;
}


/**
  * @brief       Select UVC video stream frame rate. The UVC device may support only 2 or 3 frame rate
  *              selection. This function selects the closest frame rate.
  *
  * @param[in]   frame_rate   Target frame rate.
  *
  * @retval      - \ref UVC_OK                  Success.
  * @retval      - \ref UVC_ERR_NO_DEVICE       UVC device not found.
  * @retval      - \ref UVC_ERR_NO_STREAM       Video stream is not found or not enabled. 
  * @retval      - \ref UVC_ERR_IVALID_PARM     Invalid parameter.
  * @retval      - \ref UVC_ERR_MEMORY_OUT      Out of memory.
  * @retval      - \ref UVC_ERR_BUSY            UVC device is busy.
  * @retval      - \ref UVC_ERR_USB_XFER        USB data transfer error.
  */
int  usbh_uvc_set_frame_rate(int frame_rate)
{
	struct v4l2_streamparm parm;
	int ret;

	if (g_uvc_vdev == NULL)
		return UVC_ERR_NO_DEVICE;
		
	if (g_cur_stream == NULL)
	{
		UVC_ERROR("usbh_uvc_set_format() may not be successfully called!\n");
		return UVC_ERR_NO_STREAM;
	}

	memset(&parm, 0, sizeof parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	uvc_v4l2_get_streamparm(g_cur_stream, &parm);

	UVC_DEBUG("Current frame rate: %d/%d\n", parm.parm.capture.timeperframe.numerator,
					parm.parm.capture.timeperframe.denominator);

	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = frame_rate;

	ret = uvc_v4l2_set_streamparm(g_cur_stream, &parm);
	if (ret < 0)
		UVC_ERROR("VIDIOC_S_PARM failed!\n");
	
	if (ret == -EINVAL)
		return UVC_ERR_IVALID_PARM;
		
	if (ret == -EBUSY)
		return UVC_ERR_BUSY;
	
	if (ret == -ENOMEM)
		return UVC_ERR_MEMORY_OUT;
		
	if (ret < 0)
		return UVC_ERR_USB_XFER;

	uvc_v4l2_get_streamparm(g_cur_stream, &parm);

	UVC_DEBUG("Frame rate set: %d/%d\n", parm.parm.capture.timeperframe.numerator,
					parm.parm.capture.timeperframe.denominator);
	return 0;
	
}


/**
  * @brief       Start video in data streaming.
  *
  * @retval      - \ref UVC_OK                  Success.
  * @retval      - \ref UVC_ERR_NO_DEVICE       UVC device not found.
  * @retval      - \ref UVC_ERR_NO_STREAM       Video stream is not found or not enabled. 
  * @retval      - \ref UVC_ERR_USB_XFER        USB data transfer error.
  * @retval      Otherwise                      USB errors.
  */
int  usbh_uvc_start_video_streaming()
{
	int  ret;
	
	if (g_uvc_vdev == NULL)
		return UVC_ERR_NO_DEVICE;
		
	if (g_cur_stream == NULL)
	{
		UVC_ERROR("usbh_uvc_set_format() may not be successfully called!\n");
		return UVC_ERR_NO_STREAM;
	}

	ret = uvc_status_start(g_uvc_vdev);
	if (ret < 0)
	{
		UVC_ERROR("Failed to start status pipe!\n");
		return UVC_ERR_USB_XFER;
	}

	return uvc_video_enable(g_cur_stream, 1);
}


/**
  * @brief       Stop video in data streaming.
  *
  * @retval      - \ref UVC_OK                  Success.
  * @retval      - \ref UVC_ERR_NO_DEVICE       UVC device not found.
  * @retval      - \ref UVC_ERR_NO_STREAM       Video stream is not found or not enabled. 
  * @retval      Otherwise                      USB errors.
  */
int  usbh_uvc_stop_video_streaming()
{
	if (g_uvc_vdev == NULL)
		return UVC_ERR_NO_DEVICE;
		
	if (g_cur_stream == NULL)
	{
		UVC_ERROR("usbh_uvc_set_format() may not be successfully called!\n");
		return UVC_ERR_NO_STREAM;
	}

	uvc_status_stop(g_uvc_vdev); 
	
	return uvc_video_enable(g_cur_stream, 0);
}


/**
  * @brief        Query if there's video-in image queued in UVC driver internal buffer.  
  *
  * @param[out]   image_buf    The image buffer pointer of queued image.
  * @param[out]   image_len    Size of the queued image.
  * @param[out]   handle       Image handle. It's used to release this image later.
  *
  * @retval      - \ref UVC_OK              Successfully get an image from UVC buffer queue.
  * @retval      - \ref UVC_NO_NEW_IMAGE    There's no images in UVC buffer queue.
  * @retval      - \ref UVC_ERR_MEMORY_OUT  Out of memory.
  * @retval      - \ref UVC_ERR_NO_DEVICE   UVC device not found.
  * @retval      - \ref UVC_ERR_NO_STREAM       Video stream is not found or not enabled. 
  */
int usbh_uvc_query_image(uint8_t **image_buf, int *image_len, int *handle)
{
	struct v4l2_buffer *v4l2_buf;
	int		ret;
	
	if (g_uvc_vdev == NULL)
		return UVC_ERR_NO_DEVICE;
		
	if (g_cur_stream == NULL)
	{
		UVC_ERROR("usbh_uvc_set_format() may not be successfully called!\n");
		return UVC_ERR_NO_STREAM;
	}

	v4l2_buf = (struct v4l2_buffer *)malloc(sizeof(*v4l2_buf));
	if (v4l2_buf == NULL)
		return UVC_ERR_MEMORY_OUT;
		
	memset((char *)v4l2_buf, 0, sizeof(*v4l2_buf));
	v4l2_buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf->memory = V4L2_MEMORY_MMAP;

	ret = uvc_dequeue_buffer(&g_cur_stream->queue, v4l2_buf, 0);
	if (ret == -EAGAIN)
	{
		free(v4l2_buf);
		return UVC_NO_NEW_IMAGE;
	}
		
	if (ret < 0)
	{
		free(v4l2_buf);
		UVC_ERROR("usbh_uvc_query_image error - %d\n", ret);
		return UVC_NO_NEW_IMAGE;
	}
		
	*image_buf = (unsigned char *)g_cur_stream->queue.mem + v4l2_buf->m.offset;
	*image_len = v4l2_buf->bytesused;
	*handle = (int)v4l2_buf; 	
	return 0;
}


/**
  * @brief       Release the image get from UVC buffer queue by usbh_uvc_query_image().
  *
  * @param[in]   handle   The image handle.
  *
  * @return      None.
  */
void usbh_uvc_release_image(int handle)
{
	struct v4l2_buffer *v4l2_buf = (struct v4l2_buffer *)handle;
	
	uvc_queue_buffer(&g_cur_stream->queue, v4l2_buf);
}	


void  uvcDumpBufferHex(UINT8 *pucBuff, INT nSize)
{
	INT  	nIdx, i;
	
	nIdx = 0;
	while (nSize > 0)
	{
		sysprintf("0x%04X  ", nIdx);
		for (i = 0; i < 16; i++)
			sysprintf("%02x ", pucBuff[nIdx + i]);
		sysprintf("  ");
		for (i = 0; i < 16; i++)
		{
			if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
				sysprintf("%c", pucBuff[nIdx + i]);
			else
				sysprintf(".");
			nSize--;
		}
		nIdx += 16;
		sysprintf("\n");
	}
	sysprintf("\n");
}


