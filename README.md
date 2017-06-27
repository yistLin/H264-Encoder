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

### Core Transform & Quantization

Reference:
* [H.264/AVC 4x4 Transform and Quantization](https://www.vcodex.com/h264avc-4x4-transform-and-quantization/)
* [H.264 學習筆記（四）—— 變換與量化(ZT)](http://blog.163.com/qingyu_1984/blog/static/144414503201242410221760)
* [H.264 中整數DCT變換，量化，反量化，反DCT究竟是如何實現的？](http://blog.csdn.net/stpeace/article/details/8119041)
* [Hadamard 變換中的歸一化問題](http://www.cnblogs.com/xkfz007/archive/2012/07/31/2616689.html)

### CAVLC

Reference:
* [CAVLC Encoding Tutorial - The Wobbly Cucumber](http://wobblycucumber.blogspot.tw/2013/12/cavlc-encoding-tutorial.html)

### Book Reference

* [Next-Generation Video Coding and Streaming](https://books.google.com.tw/books?id=Sa-RCwAAQBAJ&printsec=frontcover&hl=zh-TW#v=onepage&q&f=false)
* [Next-Generation Video Coding and Streaming -- Wiley Online Library](http://onlinelibrary.wiley.com/book/10.1002/9781119133346)
* [新一代視頻壓縮編碼標準H.264](http://read.pudn.com/downloads139/ebook/600638/%E6%96%B0%E4%B8%80%E4%BB%A3%E8%A7%86%E9%A2%91%E5%8E%8B%E7%BC%A9%E7%BC%96%E7%A0%81%E6%A0%87%E5%87%86H[1].264(%E6%AF%95%E5%8E%9A%E6%9D%B0).pdf)
