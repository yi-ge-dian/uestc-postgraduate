# GeekGFS
A Simple Implementation For GFS（Google File System）

GeekGFS具有客户端 client、元数据服务器 master 和多个数据服务器 chunkserver 三个基本组件

## GFSClient
GFSClient需要实现 read、write、append、exist 和 delete 函数功能。
函数实现要求：从 master 获取元数据（包含 chunk ID 和 chunk 位置），更新 master 上的元数据，最后与 chunkserver 进行实际的数据传输。

## GFSMasterServer
GFSMasterserver 模拟 GFS 元数据服务器，在内存中存储所有的元数据，且需要周期备份到硬盘。

## GFSChunkServer
GFSChunkserver实现分块存储，块固定为 64KB。为简化起见，可使用同一个节点的多个路径模拟多个物理节点。

编写测试程序，进行测试验证。开发语言不限。

> NOTE:可不考虑元数据加锁、chunk租约、复制，master故障转移、chunkserver心跳、垃圾回收等复杂机制。


PS：本项目为2022年分布式系统大作业，2022级同学不要COPY，我已经把项目链接附到了实验报告中。