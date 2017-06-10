# MPEG Encoder

NTU CSIE ITCT 2017 Spring Final Project

### Benchmark Video

To create and playback a raw video, we use *ffmpeg* and *ffplay*.

```bash
ffmpeg -i input.mp4 -vcodec rawvideo -pix_fmt yuv420p output.yuv  # generate raw video
```

```bash
ffplay -f rawvideo -pix_fmt yuv420p -video_size 720x480 -i raw.yuv  # playback raw video
```

To convert from YUV file to H.264 format file, we use *ffmpeg* again.

```bash
ffmpeg -f rawvideo -pix_fmt yuv420p -s:v 854x480 -r 25 -i input.yuv -c:v libx264 -f rawvideo output.264
```

### YUV File

A file with extension *.yuv* is YUV file. It does not contain any header. For example, *snoopy.yuv* is a YUV file with resolution 854x480 and 4:2:0 ratio (yuv420p), which means each frame starts with 854x480 pixels of Y followed by 427x240 pixels of U and followed by 427x240 pixels of V. That is, a single frame of *snoopy.yuv* contains 614880 pixels.

