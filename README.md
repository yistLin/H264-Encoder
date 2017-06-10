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

