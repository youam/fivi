# fivi - filter viewer

There are a range of tools showing you progress and throughput between pipe
commands. All of those tools only show you a single point's measurements.

When you want to know not only how much data your compression tool already ate,
but also how large the resulting file is going to be, and what the change rate
between input and output is, fivi can help you:

```
$ fivi gzip -9 < debian-9.0.0-amd64-netinst.iso > debian-9.0.0-amd64-netinst.iso.gz
ELA:   2.0s |  11.4s  in:   43M @  21MB/s ( 14.99% of  290M) out:   41M @  20MB/s ( 94.39% |  273M)
```
