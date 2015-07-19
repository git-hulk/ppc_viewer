# fip_viewer

### 1） fip_viewer 是干什么的? 

```
1. 查看进程打开了哪些文件

2. 查看进程 pagecache 的使用情况, 同时可以看单个文件的 pagecahce.
```

[Github代码](https://github.com/git-hulk/fip_viewer)

### 2) 怎么用

```
$ git clone https://github.com/git-hulk/fip_viewer.git
$ cd fip_viewer/src
$ make
$ ./fip_viewer -p pid 
```

#### 2.1) 支持选项

```
-p pid 必选，选择查看的pid

-d 可选, 详细模式，会打印当前进程打开的文件, 对应的pagecache使用情况

-l 可选, 查看当前进程打开的文件

-h 可选, 打印帮助
```

#### 2.2) 简单打印输出

![image](http://hulkdev-hulkimgs.stor.sinaapp.com/imgs/fip_viewer.jpeg)


### 3） TODO


> 1. 后台运行支持 
> 2. 支持正则过滤查看文件
> 3. 长期的 bug 修复

