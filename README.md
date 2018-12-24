## Youtube-DL Frontend

https://www.youtube.com/watch?v=9xNf79ybfLk

This is a minimal GUI wrapper around the command line tool
Youtube-DL.


![MediaDownloader Preview](https://i.imgur.com/7Xy4L3e.png "MediaDownloader Preview")

## Features

- Configure the concurrent download count
- Let the application watch your clippboard and add links automatically
- Batch add links from your clippboard
- Extract only audio (mp3) (needs ffmpeg, see youtube-dl documentation for more information)
- Configure save location globally or individually per container/song
- Save containers into subfolders

## Compiling

As this is written with the QT-Framework, make sure you 
are able to compile qt-applications before.

- qmake . && make

## Requirements

- You need to have the youtube-dl binary in your path

You can see a preview on youtube: [https://www.youtube.com/watch?v=9xNf79ybfLk](https://www.youtube.com/watch?v=9xNf79ybfLk)