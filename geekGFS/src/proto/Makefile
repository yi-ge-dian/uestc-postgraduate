gen:
	protoc --go_out=../pb --go_opt=paths=source_relative --go-grpc_out=../pb --go_opt=paths=source_relative *.proto

clean:
	rm -f ../pb/*.go