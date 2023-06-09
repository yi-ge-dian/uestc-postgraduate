package masterserver

type Chunk struct {
	locations []string
}

func NewChunk(locations []string) *Chunk {
	return &Chunk{locations: locations}
}
