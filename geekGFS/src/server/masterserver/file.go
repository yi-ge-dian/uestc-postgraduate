package masterserver

type File struct {
	FilePath       string            `json:"filePath"`       // 文件路径（命名空间）
	Chunks         map[string]*Chunk `json:"chunks"`         // key:chunkHandle value:Chunk
	ChunkHandleSet []string          `json:"chunkHandleSet"` // 存储chunkHandle的集合
}

func NewFile(filePath *string) *File {
	return &File{
		FilePath: *filePath,
		Chunks:   make(map[string]*Chunk, 0),
	}
}

func (f *File) GetChunks() *map[string]*Chunk {
	return &f.Chunks
}
