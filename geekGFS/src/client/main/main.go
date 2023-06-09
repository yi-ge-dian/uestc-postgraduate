package main

import (
	cl "GeekGFS/src/client"
	"GeekGFS/src/pb"
	"context"
	"github.com/sadlil/gologger"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"os"
	"time"
)

// 日志库，六个级别，Log、Message、Info、Warn、Debug、Error
// 服务器运行情况: Info、Error()
// 服务器交流信息: Message、Warn
// 服务器查找Bug: Debug

func printUsage() {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	logger.Info("Usage: ")
	logger.Info("<command> " + "<filePath> " + "<args>(optional)")
	logger.Info("create filePath")
	logger.Info("  list filePath")
	logger.Info(" write filePath data")
	logger.Info("  read filePath")
	logger.Info("append filePath data")
	logger.Info("delete filePath")
}

func main() {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	if len(os.Args) < 3 {
		logger.Warn("Input too few parameters, at least 3, please refer to Usage to use")
		return
	}

	// 1. 建立连接，端口是服务端开放的30001端口 没有证书会报错
	masterServerSocket := "127.0.0.1:30001"
	conn, err := grpc.Dial(masterServerSocket, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		logger.Error(err.Error())
	}
	logger.Info("Client connected masterServer at " + masterServerSocket)
	// 退出时关闭链接
	defer func(conn *grpc.ClientConn) {
		err := conn.Close()
		if err != nil {
			logger.Error("Failed to close the connection" + err.Error())
		}
	}(conn)

	// 2. 创建一个 client for masterServer
	clientForMS := pb.NewMasterServerToClientClient(conn)
	clientForMSCtx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	// 3. 根据 command 调用方法
	command := os.Args[1]
	filePath := os.Args[2]
	switch command {
	case "create":
		if len(os.Args) != 3 {
			logger.Warn("Create only needs two arguments")
			return
		}
		cl.CreateFile(&clientForMS, &clientForMSCtx, &filePath)
	case "list":
		if len(os.Args) != 3 {
			logger.Warn("List only needs two arguments")
			return
		}
		cl.ListFiles(&clientForMS, &clientForMSCtx, &filePath)
	case "write":
		if len(os.Args) < 4 {
			logger.Warn("Write at least needs 3 arguments")
			return
		}
		data := os.Args[3]
		for i := 4; i < len(os.Args); i++ {
			data = data + " " + os.Args[i]
		}
		cl.WriteFile(&clientForMS, &clientForMSCtx, &filePath, &data)
	case "read":
		if len(os.Args) != 3 {
			logger.Warn("Read only needs 3 arguments")
		}
		cl.ReadFile(&clientForMS, &clientForMSCtx, &filePath)
	case "append":
		if len(os.Args) < 4 {
			logger.Warn("Append at least needs 3 arguments")
			return
		}
		data := os.Args[3]
		for i := 4; i < len(os.Args); i++ {
			data = data + " " + os.Args[i]
		}
		cl.AppendFile(&clientForMS, &clientForMSCtx, &filePath, &data)
	case "delete":
		if len(os.Args) != 3 {
			logger.Warn("Read only needs 3 arguments")
		}
		cl.DeleteFile(&clientForMS, &clientForMSCtx, &filePath)
	default:
		printUsage()
	}
}
