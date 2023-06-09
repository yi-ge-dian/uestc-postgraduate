package masterserver

import (
	cm "GeekGFS/src/common"
	"math/rand"
	"time"
)

type MetaData struct {
	Locations []string         `json:"locations"` //
	Files     map[string]*File `json:"files"`     // key:filepath  value:File
}

//************************************辅助函数************************************

func NewMetaData(locations []string) *MetaData {
	return &MetaData{
		Files:     make(map[string]*File, 0),
		Locations: locations,
	}
}

// GetFiles 获得所有的文件
func (md *MetaData) GetFiles() *map[string]*File {
	return &md.Files
}

// ChooseChunkServerLocations 选择存放 chunkServer的位置
func (md *MetaData) ChooseChunkServerLocations(chunkServerLocations *[]string) {
	gfsConfig := cm.NewGFSConfig(cm.GFSChunkSize, cm.GFSChunkServerLocations, cm.GFSChunkServer)
	total := len(gfsConfig.ChunkServerLocations())
	rand.Seed(time.Now().Unix())
	index := rand.Int()
	for i := 0; i < 3; i++ {
		*chunkServerLocations = append(*chunkServerLocations, gfsConfig.ChunkServerLocations()[(index+i)%total])
	}
}

// GetLatestChunkHandle 得到最新的chunkHandle
func (md *MetaData) GetLatestChunkHandle(filePath *string) string {
	// 如果找到了该文件
	if file, ok := md.Files[*filePath]; ok {
		if len(file.ChunkHandleSet) > 0 {
			return file.ChunkHandleSet[len(file.ChunkHandleSet)-1]
		}
	}
	return "-2"
}

//*********************************** 业务函数 ************************************

// CreateNewFile 创建新文件
func (md *MetaData) CreateNewFile(filePath *string, chunkHandle *string, statusCode *cm.StatusCode) {
	// 生成 chunkHandle
	*chunkHandle = cm.GenerateChunkHandle()
	// 如果已经创建该文件
	if _, ok := md.Files[*filePath]; ok {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: File exists already" + *filePath
		return
	}
	// 插入
	file := NewFile(filePath)
	md.Files[*filePath] = file
	// 创建chunk
	*chunkHandle = "1" + *chunkHandle
	prevChunkHandle := "-1"
	md.CreateNewChunk(filePath, &prevChunkHandle, chunkHandle, statusCode)
}

// CreateNewChunk 创建新的chunk
func (md *MetaData) CreateNewChunk(filePath *string, prevChunkHandle *string, chunkHandle *string, statusCode *cm.StatusCode) {
	file, ok := md.Files[*filePath]
	// 如果该文件未被创建，就来创建 chunk 是不科学的
	if !ok {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: New chunk but file doesn't exist:" + *filePath
		return
	}
	// chunk不是新建的
	latestChunkHandle := ""
	if *prevChunkHandle != "-1" {
		latestChunkHandle = md.GetLatestChunkHandle(filePath)
	}
	// chunk 不是新建的，并且与该文件最新的 chunkHandle 对不上号
	if *prevChunkHandle != "-1" && latestChunkHandle != *prevChunkHandle {
		statusCode.Value = "-1"
		statusCode.Exception = "ERROR: New chunk already created: " + *filePath + ":" + *chunkHandle
		return
	}
	// 选择chunk的位置
	var chunkServerLocations []string
	md.ChooseChunkServerLocations(&chunkServerLocations)
	newChunk := NewChunk(chunkServerLocations)
	chunks := file.GetChunks()
	(*chunks)[*chunkHandle] = newChunk
	// 更新文件信息
	file.ChunkHandleSet = append(file.ChunkHandleSet, *chunkHandle)
	// 设置状态码
	statusCode.Value = "0"
	statusCode.Exception = "SUCCESS: New Chunk Created"
}

// DeleteFile 删除文件
func (md *MetaData) DeleteFile(filePath *string) {
	files := md.GetFiles()
	delete(*files, *filePath)
}
