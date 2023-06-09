package common

import "github.com/gofrs/uuid"

type StatusCode struct {
	Value     string
	Exception string
}

const (
	GFSChunkSize   = 64
	GFSChunkServer = "ChunkServer"
)

var (
	GFSChunkServerLocations = []string{"30002", "30003", "30004", "30005", "30006"}
)

// GenerateChunkHandle 生成chunkHandle号,形式：5b912ae9-71c1-464d-8e32-712b4b506430
func GenerateChunkHandle() string {
	uid, _ := uuid.NewV4()
	return uid.String()
}
