package chunkserver

import (
	cm "GeekGFS/src/common"
	"GeekGFS/src/pb"
	"bufio"
	"context"
	"fmt"
	"github.com/sadlil/gologger"
	"io"
	"os"
	"strconv"
	"strings"
)

type ChunkServer struct {
	pb.UnimplementedChunkServerToClientServer
	port string
	root string
}

// ************************************ 辅助函数 ************************************************************************

func NewChunkServer(port *string, root string) *ChunkServer {
	var cs ChunkServer
	cs.port = *port
	cs.root = "./" + root + "/" + *port
	err := os.MkdirAll(cs.root, os.ModePerm)
	if err != nil {
		return nil
	}
	return &cs
}

func CheckFileExist(path string) bool {
	_, err := os.Lstat(path)
	return !os.IsNotExist(err)
}

// OpenFileWithCreate 打开文件(没有就创建)
func (cs *ChunkServer) OpenFileWithCreate(filePath *string, statusCode *cm.StatusCode) *os.File {
	file, err := os.OpenFile(*filePath, os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: " + err.Error()
		return nil
	}
	return file
}

// OpenFileWithAppend 打开文件(已经存在，想要追加)
func (cs *ChunkServer) OpenFileWithAppend(filePath *string, statusCode *cm.StatusCode) *os.File {
	file, err := os.OpenFile(*filePath, os.O_RDWR|os.O_APPEND, 0666)
	if err != nil {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: " + err.Error()
		return nil
	}
	return file
}

// ReadFile 读取文件
func (cs *ChunkServer) ReadFile(file *os.File, data *string, statusCode *cm.StatusCode) {
	reader := bufio.NewReader(file)
	chunks := make([]byte, 0)
	buf := make([]byte, 1024)
	for {
		n, err := reader.Read(buf)
		if err != nil && err != io.EOF {
			panic(err)
		}
		if 0 == n {
			break
		}
		chunks = append(chunks, buf[:n]...)
	}
	*data = string(chunks)
}

// WriteFile 写文件
func (cs *ChunkServer) WriteFile(file *os.File, data *string, statusCode *cm.StatusCode) {
	// 带缓存的Writer
	write := bufio.NewWriter(file)
	_, err := write.WriteString(*data)
	if err != nil {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: " + err.Error()
		return
	}
	// 冲到磁盘中
	err = write.Flush()
	if err != nil {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: " + err.Error()
		return
	}
}

// ************************************ 业务函数 *************************************************************************

// Create 创建
func (cs *ChunkServer) Create(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	chunkHandle := req.SendMessage
	logger.Message(cs.port + " Create Chunk " + chunkHandle)
	// 定义变量，传进去
	var statusCode cm.StatusCode
	// 核心逻辑
	cs.create(&chunkHandle, &statusCode)
	// 返回信息给客户端
	return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
}

// create 核心逻辑
func (cs *ChunkServer) create(chunkHandle *string, statusCode *cm.StatusCode) {
	filePath := cs.root + "/" + *chunkHandle
	// 打开文件
	file := cs.OpenFileWithCreate(&filePath, statusCode)
	// 关闭文件
	err := file.Close()
	if err != nil {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: " + err.Error()
		return
	}
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: Chunk Created"
}

// Write 写
func (cs *ChunkServer) Write(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	slice := strings.Split(req.SendMessage, "|")
	chunkHandle := slice[0]
	data := slice[1]
	logger.Message(cs.port + " Write Chunk " + data + " to " + chunkHandle)
	// 定义变量，传进去
	var statusCode cm.StatusCode
	// 核心逻辑
	cs.write(&chunkHandle, &data, &statusCode)
	// 返回逻辑
	return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
}

// write 核心逻辑
func (cs *ChunkServer) write(chunkHandle *string, data *string, statusCode *cm.StatusCode) {
	filePath := cs.root + "/" + *chunkHandle
	if !CheckFileExist(filePath) {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: File not exists, please create one"
		return
	}
	// 打开文件
	file := cs.OpenFileWithCreate(&filePath, statusCode)
	// 关闭文件
	defer func(file *os.File) {
		err := file.Close()
		if err != nil {
			statusCode.Value = "-1"
			statusCode.Exception = "ERROR: " + err.Error()
			return
		}
	}(file)
	// 写入文件
	cs.WriteFile(file, data, statusCode)
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: " + cs.port + " write data into " + *chunkHandle
}

// Read 读
func (cs *ChunkServer) Read(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	chunkHandle := req.SendMessage
	logger.Message(cs.port + " Read data from " + chunkHandle)
	// 定义变量传进去
	data := ""
	var statusCode cm.StatusCode
	// 核心逻辑
	cs.read(&chunkHandle, &data, &statusCode)
	// 返回逻辑
	if statusCode.Value != "0" {
		return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
	}
	return &pb.Reply{ReplyMessage: data, StatusCode: statusCode.Value}, nil
}

// read 核心逻辑
func (cs *ChunkServer) read(chunkHandle *string, data *string, statusCode *cm.StatusCode) {
	filePath := cs.root + "/" + *chunkHandle
	if !CheckFileExist(filePath) {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: File not exists, please create one"
		return
	}
	file := cs.OpenFileWithCreate(&filePath, statusCode)
	defer func(file *os.File) {
		err := file.Close()
		if err != nil {
			statusCode.Value = "-1"
			statusCode.Exception = "ERROR: " + err.Error()
			return
		}
	}(file)
	// 读取文件
	fmt.Println(file)
	cs.ReadFile(file, data, statusCode)
	fmt.Println(*data)
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: " + cs.port + " read data from " + *chunkHandle
}

// GetChunkSpace 获得 chunk 的空间
func (cs *ChunkServer) GetChunkSpace(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	chunkHandle := req.SendMessage
	logger.Message(cs.port + " Get ChunkRemainSize of " + chunkHandle)
	// 定义变量传进去
	chunkRemainSize := 0
	var statusCode cm.StatusCode
	// 核心逻辑
	cs.getChunkSpace(&chunkHandle, &chunkRemainSize, &statusCode)
	// 返回逻辑
	if statusCode.Value != "0" {
		return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
	}
	return &pb.Reply{ReplyMessage: strconv.Itoa(chunkRemainSize), StatusCode: statusCode.Value}, nil
}

// getChunkSpace 核心逻辑
func (cs *ChunkServer) getChunkSpace(chunkHandle *string, chunkRemainSize *int, statusCode *cm.StatusCode) {
	filePath := cs.root + "/" + *chunkHandle
	if !CheckFileExist(filePath) {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: File not exists, please create one"
		return
	}
	// 打开文件
	file := cs.OpenFileWithCreate(&filePath, statusCode)
	// 关闭文件
	defer func(file *os.File) {
		err := file.Close()
		if err != nil {
			statusCode.Value = "-1"
			statusCode.Exception = "ERROR: " + err.Error()
			return
		}
	}(file)
	// 读取文件
	data := ""
	cs.ReadFile(file, &data, statusCode)
	*chunkRemainSize = len(data)
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: " + cs.port + " get remain size of " + filePath
}

// Append 追加
func (cs *ChunkServer) Append(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	slice := strings.Split(req.SendMessage, "|")
	chunkHandle := slice[0]
	data := slice[1]
	logger.Message(cs.port + "Append Chunk " + data + " to " + chunkHandle)
	// 定义变量，传进去
	var statusCode cm.StatusCode
	// 核心逻辑
	cs.append(&chunkHandle, &data, &statusCode)
	// 返回逻辑
	return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
}

// append 核心逻辑
func (cs *ChunkServer) append(chunkHandle *string, data *string, statusCode *cm.StatusCode) {
	filePath := cs.root + "/" + *chunkHandle
	if !CheckFileExist(filePath) {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: File not exists, please create one"
		return
	}
	// 打开文件
	file := cs.OpenFileWithAppend(&filePath, statusCode)
	// 关闭文件
	defer func(file *os.File) {
		err := file.Close()
		if err != nil {
			statusCode.Value = "-1"
			statusCode.Exception = "ERROR: " + err.Error()
			return
		}
	}(file)
	// 写入文件
	cs.WriteFile(file, data, statusCode)
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: " + cs.port + " append data of " + filePath
}
