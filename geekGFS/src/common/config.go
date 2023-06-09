package common

// GFSConfig 配置
type GFSConfig struct {
	chunkSize            int
	chunkServerLocations []string
	chunkServerRoot      string
}

func NewGFSConfig(chunkSize int, chunkServerLocations []string, chunkServerRoot string) *GFSConfig {
	return &GFSConfig{
		chunkSize:            chunkSize,
		chunkServerLocations: chunkServerLocations,
		chunkServerRoot:      chunkServerRoot,
	}
}

func (g *GFSConfig) ChunkServerLocations() []string {
	return g.chunkServerLocations
}
