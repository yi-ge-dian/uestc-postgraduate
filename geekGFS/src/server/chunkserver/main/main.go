package main

import (
	cm "GeekGFS/src/common"
	"GeekGFS/src/pb"
	cs "GeekGFS/src/server/chunkserver"
	"github.com/sadlil/gologger"
	"google.golang.org/grpc"
	"net"
	"sync"
)

// 日志库，六个级别，Log、Message、Info、Warn、Debug、Error
// 服务器运行情况:Info、Error
// 服务器交流信息：Message、Warn
// 服务器查找Bug：Debug

func StartChunkServer(chunkServerLocation *string, done func()) {
	defer done()
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	// 1. chunkServer配置
	chunkServerSocket := "127.0.0.1:" + *chunkServerLocation
	chunkServer := cs.NewChunkServer(chunkServerLocation, cm.GFSChunkServer)

	// 2. grpc服务器
	s := grpc.NewServer()

	// 3. 注册服务器端服务
	pb.RegisterChunkServerToClientServer(s, chunkServer)
	listener, err := net.Listen("tcp", chunkServerSocket)
	if err != nil {
		logger.Error("Failed to listen to the port" + err.Error())
	}
	defer func(listener net.Listener) {
		err_ := listener.Close()
		if err_ != nil {
			logger.Error("Failed to close the port")
		}
	}(listener)

	// 4. 启动监听
	logger.Info("chunkServer listening at " + listener.Addr().String())
	_ = s.Serve(listener)
}

func main() {
	var wg sync.WaitGroup
	wg.Add(5)
	for i := 0; i < 5; i++ {
		go StartChunkServer(&cm.GFSChunkServerLocations[i], wg.Done)
	}
	wg.Wait()
}
