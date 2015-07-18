# fip_viewer

查看进程打开的文件以及pagecache使用情况

## 功能

1. 根据 pid 查看进程打开文件的情况

```
[root@hzwy_1_12 src]# ./fip_viewer -d -p 8258
/var/run/zabbix/zabbix_agentd.pid    page_in_mem:0  total_pages:1
```

2. 根据 pid 查看进程每个文件以及整体使用 pagecache 的情况

```
[root@hzwy_1_12 src]# ./fip_viewer -d -p 8258
/var/run/zabbix/zabbix_agentd.pid    page_in_mem:0  total_pages:1

========================== SUMMARY ==========================
SUMMARY: total size:4.00K, page in mem: 0B, ratio: 0.00000%
========================== SUMMARY ==========================
```

## TODO

1. 支持根据正则过滤文件

2. 部分 bug 修复

