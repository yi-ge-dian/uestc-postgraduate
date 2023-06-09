package main

import (
	cm "GeekGFS/src/common"
	"GeekGFS/src/pb"
	ms "GeekGFS/src/server/masterserver"
	"github.com/sadlil/gologger"
	"google.golang.org/grpc"
	"net"
	"time"
)

// 日志库，六个级别，Log、Message、Info、Warn、Debug、Error
// 服务器运行情况:Info、Error
// 服务器交流信息：Message、Warn
// 服务器查找Bug：Debug

func main() {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)

	// 1. masterServer配置
	masterServerSocket := "127.0.0.1:30001"
	gfsConfig := cm.NewGFSConfig(cm.GFSChunkSize, cm.GFSChunkServerLocations, cm.GFSChunkServer)
	masterServer := ms.NewMasterServer(&masterServerSocket, gfsConfig.ChunkServerLocations())

	// 2. grpc服务器
	s := grpc.NewServer()

	// 3. 注册服务器端服务
	pb.RegisterMasterServerToClientServer(s, masterServer)
	listener, err := net.Listen("tcp", masterServerSocket)
	if err != nil {
		logger.Error("Failed to listen to the port" + err.Error())
	}
	defer func(listener net.Listener) {
		err_ := listener.Close()
		if err_ != nil {
			logger.Error("Failed to close the port")
		}
	}(listener)

	// 5. 定期存储元数据信息
	ticker := time.NewTicker(30 * time.Second)
	go func() {
		for {
			<-ticker.C
			masterServer.Storage()
		}
	}()

	// 4. 启动监听
	logger.Info("masterServer listening at " + listener.Addr().String())
	_ = s.Serve(listener)
}
