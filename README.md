# MPEG Encoder

NTU CSIE ITCT 2017 Spring Final Project

### Benchmark Video

To create and playback a raw video, we use *ffmpeg* and *ffplay*.

```bash
ffmpeg -i input.mp4 -vcodec rawvideo -pix_fmt rgb24 output.rgb  # generate raw video
```

```bash
ffplay -f rawvideo -pix_fmt rgb24 -video_size 720x480 -i raw.rgb  # playback raw video
```

To convert from RGB file to H.264 format file, we use *ffmpeg* again.

```bash
ffmpeg -f rawvideo -pix_fmt rgb24 -s:v 854x480 -r 25 -i input.yuv -c:v libx264 -f rawvideo output.264
```

### RGB File

A file with extension *.rgb* is an RGB file. It does not contain any header. For example, *snoopy.rgb* is a RGB file with resolution 854x480, and each pixels contains 1 byte of red followed by 1 byte of green and followed by 1 byte of blue.

### Intra Prediction

There are 2 sets of intra prediction modes, one is 16x16 luma prediction modes, and the other is 8x8 chroma prediction modes.

Reference:
* [H.264/AVC Intra Prediction](https://www.vcodex.com/h264avc-intra-precition/)
* [Fast Intra-Prediction Mode Selection for H.264](https://pdfs.semanticscholar.org/7eba/7f2d55ffc7e74d5b7eae7cd30f4a9038581a.pdf)
