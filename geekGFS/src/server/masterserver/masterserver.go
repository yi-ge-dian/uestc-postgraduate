package masterserver

import (
	cm "GeekGFS/src/common"
	"GeekGFS/src/pb"
	"bufio"
	"context"
	"encoding/json"
	"github.com/sadlil/gologger"
	"os"
	"strconv"
	"strings"
	"time"
)

type MasterServer struct {
	pb.UnimplementedMasterServerToClientServer `json:"-"`
	Port                                       string   `json:"port"`
	Metadata                                   MetaData `json:"metadata"`
}

//************************************辅助函数************************************

// NewMasterServer 构造函数
func NewMasterServer(port *string, locations []string) *MasterServer {
	return &MasterServer{Port: *port, Metadata: *NewMetaData(locations)}
}

// checkValidFile 检查文件是否有效
func (ms *MasterServer) checkValidFile(filePath *string, statusCode *cm.StatusCode) {
	files := ms.Metadata.GetFiles()
	// 如果该文件不存在
	if _, ok := (*files)[*filePath]; !ok {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: File doesn't exist in " + *filePath
	} else {
		statusCode.Value = "0"
		statusCode.Exception = "SUCCESS: File exists in " + *filePath
	}
}

//*********************************** 业务函数 ************************************

// CreateFile 创建文件
func (ms *MasterServer) CreateFile(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	filePath := req.SendMessage
	logger.Message("Command CreateFile " + filePath)
	// 定义变量，传进去
	var chunkHandle string
	var locations []string
	var statusCode cm.StatusCode
	// 核心逻辑
	ms.createFile(&filePath, &chunkHandle, &locations, &statusCode)
	// 打印状态
	switch statusCode.Value {
	case "0":
		logger.Message(statusCode.Exception)
	default:
		logger.Warn(statusCode.Exception)
	}
	// 返回信息给客户端
	if statusCode.Value != "0" {
		return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
	}
	replyMessage := chunkHandle
	for i := 0; i < len(locations); i++ {
		replyMessage = replyMessage + "|" + locations[i]
	}
	return &pb.Reply{ReplyMessage: replyMessage, StatusCode: statusCode.Value}, nil
}

// createFile 核心逻辑
func (ms *MasterServer) createFile(filePath *string, chunkHandle *string, locations *[]string, statusCode *cm.StatusCode) {
	// 元数据信息需要修改
	ms.Metadata.CreateNewFile(filePath, chunkHandle, statusCode)
	// 返回码错误
	if statusCode.Value != "0" {
		return
	}
	// 返回码正确
	files := ms.Metadata.GetFiles()
	if file, ok := (*files)[*filePath]; ok {
		chunks := file.GetChunks()
		chunk := (*chunks)[*chunkHandle]
		*locations = chunk.locations
	}
}

// ListFiles 展示文件列表
func (ms *MasterServer) ListFiles(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	filePath := req.SendMessage
	logger.Message("Command ListFiles " + filePath)
	// 定义变量，传进去
	var filePaths []string
	// 核心逻辑
	ms.listFiles(&filePath, &filePaths)
	// 返回信息给客户端
	var statusCode cm.StatusCode
	replyMessage := ""
	if len(filePaths) == 0 {
		statusCode.Value = "-1"
		statusCode.Exception = filePath + ":" + "is not exist"
		return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
	}
	for _, file := range filePaths {
		replyMessage = replyMessage + "|" + file
	}
	return &pb.Reply{ReplyMessage: replyMessage, StatusCode: "0"}, nil
}

// listFiles 核心逻辑
func (ms *MasterServer) listFiles(filePath *string, filePaths *[]string) {
	for k, _ := range ms.Metadata.Files {
		// 长度太小，直接下次循环
		if len(k) < len(*filePath) {
			continue
		}
		// 截取子串,符合条件的加入切片中,切片是头闭尾开
		subK := k[0:len(*filePath)]
		if strings.Compare(subK, *filePath) == 0 {
			*filePaths = append(*filePaths, k)
		}
	}
}

// WriteFile 写文件
func (ms *MasterServer) WriteFile(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	// 分割串
	slice := strings.Split(req.SendMessage, "|")
	filePath := slice[0]
	data := slice[1]
	logger.Message("Command WriteFile " + data + "to " + filePath)
	// 定义变量，传进去
	var statusCode cm.StatusCode
	var chunksLocations string
	// 核心逻辑
	ms.writeFile(&filePath, &data, &chunksLocations, &statusCode)
	// 返回信息给客户端
	if statusCode.Value != "0" {
		return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
	}
	return &pb.Reply{ReplyMessage: chunksLocations, StatusCode: statusCode.Value}, nil
}

// writeFile 核心逻辑
func (ms *MasterServer) writeFile(filePath *string, data *string, chunksLocations *string, statusCode *cm.StatusCode) {
	// 检查文件是否有效
	ms.checkValidFile(filePath, statusCode)
	if statusCode.Value != "0" {
		return
	}
	// 新加的 data 有多少chunk
	var chunkNum int
	if len(*data)%cm.GFSChunkSize != 0 {
		chunkNum = len(*data)/cm.GFSChunkSize + 1
	} else {
		chunkNum = len(*data) / cm.GFSChunkSize
	}
	// 先拿到文件
	files := ms.Metadata.GetFiles()
	file := (*files)[*filePath]
	// 获得原始的 chunk 数量
	prevChunkNum := len(file.ChunkHandleSet)
	// 如果新加的 chunkNum 比 原来的chunkNum 小
	if chunkNum <= prevChunkNum {
		// 拿到前面的位置
		for i := 0; i < chunkNum; i++ {
			chunkHandle := file.ChunkHandleSet[i]
			*chunksLocations = (*chunksLocations) + "|" + chunkHandle
			chunks := file.GetChunks()
			chunk := (*chunks)[chunkHandle]
			for _, location := range chunk.locations {
				*chunksLocations = (*chunksLocations) + "|" + location
			}
		}
		// 移除后面的 chunk
		for i := chunkNum; i < len(file.ChunkHandleSet); i++ {
			chunkHandle := file.ChunkHandleSet[i]
			chunks := file.GetChunks()
			delete(*chunks, chunkHandle)
		}
		// 更新 chunkHandleSet
		file.ChunkHandleSet = file.ChunkHandleSet[0:chunkNum]
	} else {
		// 新加的需要的 chunk 数目 比 原来的 chunk 数目大
		// 直接拿走所有的原来的chunk 位置
		for i := 0; i < prevChunkNum; i++ {
			chunkHandle := file.ChunkHandleSet[i]
			*chunksLocations = (*chunksLocations) + "|" + chunkHandle
			chunks := file.GetChunks()
			chunk := (*chunks)[chunkHandle]
			for _, location := range chunk.locations {
				*chunksLocations = (*chunksLocations) + "|" + location
			}
		}
		// 需要新加得的 chunk 数目
		addChunkNum := chunkNum - prevChunkNum
		for addChunkIndex := 1; addChunkIndex <= addChunkNum; addChunkIndex++ {
			chunkHandle := strconv.Itoa(addChunkIndex+prevChunkNum) + cm.GenerateChunkHandle()
			*chunksLocations = *chunksLocations + "|" + chunkHandle
			var chunkServerLocations []string
			ms.Metadata.ChooseChunkServerLocations(&chunkServerLocations)
			for _, location := range chunkServerLocations {
				*chunksLocations = *chunksLocations + "|" + location
			}
			newChunk := NewChunk(chunkServerLocations)
			chunks := file.GetChunks()
			(*chunks)[chunkHandle] = newChunk
			file.ChunkHandleSet = append(file.ChunkHandleSet, chunkHandle)
		}
	}
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: chunksLocations " + *chunksLocations
}

// ReadFile 读取文件
func (ms *MasterServer) ReadFile(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	filePath := req.SendMessage
	logger.Message("Command ReadFile " + filePath)
	// 定义变量，传进去
	data := ""
	var statusCode cm.StatusCode
	// 核心逻辑
	ms.readFile(&filePath, &data, &statusCode)
	// 返回信息给客户端
	if statusCode.Value != "0" {
		return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
	}
	return &pb.Reply{ReplyMessage: data, StatusCode: statusCode.Value}, nil
}

// readFile 核心逻辑
func (ms *MasterServer) readFile(filePath *string, data *string, statusCode *cm.StatusCode) {
	// 校验文件有效性
	ms.checkValidFile(filePath, statusCode)
	// 返回码错误
	if statusCode.Value != "0" {
		return
	}
	// 先拿到文件
	files := ms.Metadata.GetFiles()
	file := (*files)[*filePath]
	chunks := file.GetChunks()
	// 拿到所有的 chunkHandle，再通过 chunkHandle 去拿 chunk
	for _, chunkHandle := range file.ChunkHandleSet {
		chunk := (*chunks)[chunkHandle]
		// 得到 chunk 存放的任意一个位置
		*data = *data + "|" + chunkHandle + "|" + chunk.locations[0]
	}
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: file " + *filePath + "is read"
}

// AppendFile 追加文件
func (ms *MasterServer) AppendFile(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	// 分割串
	slice := strings.Split(req.SendMessage, "|")
	filePath := slice[0]
	data := slice[1]
	logger.Message("Command AppendFile " + data + "to " + filePath)
	// 定义变量，传进去
	var statusCode cm.StatusCode
	var locations []string
	// 得到最后一块 chunkHandle
	lastChunkHandle := ms.Metadata.GetLatestChunkHandle(&filePath)
	// 核心逻辑
	ms.appendFile(&filePath, &lastChunkHandle, &locations, &statusCode)
	// 返回信息给客户端
	if statusCode.Value != "0" {
		return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
	}
	reply := lastChunkHandle
	for _, location := range locations {
		reply = reply + "|" + location
	}
	return &pb.Reply{ReplyMessage: reply, StatusCode: statusCode.Value}, nil
}

// appendFile 核心逻辑
func (ms *MasterServer) appendFile(filePath *string, lastChunkHandle *string, locations *[]string, statusCode *cm.StatusCode) {
	// 校验文件有效性
	ms.checkValidFile(filePath, statusCode)
	// 返回码错误
	if statusCode.Value != "0" {
		return
	}
	files := ms.Metadata.GetFiles()
	file := (*files)[*filePath]
	chunks := file.GetChunks()
	chunk := (*chunks)[*lastChunkHandle]
	*locations = chunk.locations
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: file " + *filePath + " last chunk get"
}

// CreateChunk 创建 chunk
func (ms *MasterServer) CreateChunk(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	// 分割串
	slice := strings.Split(req.SendMessage, "|")
	filePath := slice[0]
	chunkHandle := slice[1]
	logger.Message("Command CreateChunk of " + filePath)
	// 定义变量，传进去
	var statusCode cm.StatusCode
	var locations []string
	// 核心逻辑
	ms.createChunk(&filePath, &chunkHandle, &locations, &statusCode)
	// 返回信息给客户端
	if statusCode.Value != "0" {
		return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
	}
	replyMessage := chunkHandle
	for i := 0; i < len(locations); i++ {
		replyMessage = replyMessage + "|" + locations[i]
	}
	return &pb.Reply{ReplyMessage: replyMessage, StatusCode: statusCode.Value}, nil
}

// createChunk 核心逻辑
func (ms *MasterServer) createChunk(filePath *string, chunkHandle *string, locations *[]string, statusCode *cm.StatusCode) {
	// 校验文件有效性
	ms.checkValidFile(filePath, statusCode)
	// 返回码错误
	if statusCode.Value != "0" {
		return
	}
	// 拿到 chunkHandle 的第一个数字，就是index
	chunkIndex := string((*chunkHandle)[0])
	// 再去搞一个新的 chunkHandle
	newChunkHandle := cm.GenerateChunkHandle()
	newChunkHandle = chunkIndex + newChunkHandle
	// 拿到文件
	file, ok := ms.Metadata.Files[*filePath]
	// 如果该文件未被创建，就来创建 chunk 是不科学的
	if !ok {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: New chunk but file doesn't exist:" + *filePath
		return
	}
	// 选择位置
	var chunkServerLocations []string
	ms.Metadata.ChooseChunkServerLocations(&chunkServerLocations)
	newChunk := NewChunk(chunkServerLocations)
	chunks := file.GetChunks()
	(*chunks)[*chunkHandle] = newChunk
	// 更新文件信息
	file.ChunkHandleSet = append(file.ChunkHandleSet, *chunkHandle)
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: File " + *filePath + " create new chunk"
}

// DeleteFile 删除
func (ms *MasterServer) DeleteFile(ctx context.Context, req *pb.Request) (*pb.Reply, error) {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	filePath := req.SendMessage
	logger.Message("Command Delete " + filePath)
	// 定义变量，传进去
	var statusCode cm.StatusCode
	// 核心逻辑
	ms.deleteFile(&filePath, &statusCode)
	// 返回给客户端
	return &pb.Reply{ReplyMessage: statusCode.Exception, StatusCode: statusCode.Value}, nil
}

// deleteFile 删除
func (ms *MasterServer) deleteFile(filePath *string, statusCode *cm.StatusCode) {
	// 校验文件有效性
	ms.checkValidFile(filePath, statusCode)
	// 返回码错误
	if statusCode.Value != "0" {
		return
	}
	// 元数据把它删除
	ms.Metadata.DeleteFile(filePath)
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: file " + *filePath + "is deleted"
}

// Storage 存储数据
func (ms *MasterServer) Storage() {
	logger := gologger.GetLogger(gologger.CONSOLE, gologger.ColoredLog)
	logger.Message("MasterServer is  storing······")
	dataBytes, err := json.Marshal(ms)
	if err != nil {
		logger.Warn("json.Marshal failed" + err.Error())
		return
	}
	// 打开文件
	dirPath := "./MasterServer/" + "3001"
	err = os.MkdirAll(dirPath, os.ModePerm)
	if err != nil {
		logger.Warn(err.Error())
		return
	}
	timeUnix := time.Now().Unix() //已知的时间戳
	formatTimeStr := time.Unix(timeUnix, 0).Format("2006-01-02 15:04:05")
	file, err_ := os.OpenFile(dirPath+"/"+formatTimeStr+".json", os.O_RDWR|os.O_CREATE, 0666)
	if err_ != nil {
		logger.Warn(err_.Error())
		return
	}
	// 关闭文件
	defer func(file *os.File) {
		err = file.Close()
		if err != nil {
			logger.Warn(err.Error())
		}
	}(file)
	// 写 buffer
	writer := bufio.NewWriter(file)
	for i := 0; i < len(string(dataBytes)); i++ {
		//调用WriteByte()按字节写入
		err = writer.WriteByte(dataBytes[i])
		if err != nil {
			logger.Warn(err.Error())
			return
		}
	}
	// 冲到硬盘
	err = writer.Flush()
	if err != nil {
		logger.Warn(err.Error())
		return
	}
}
