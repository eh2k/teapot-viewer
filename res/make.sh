#mv gobin_go gobin.go
#for f in *.png; do
# var=${f//[-.]/_}
# var=${var^^}
# go run gobin.go -var $var -package icons $f > res_${f%.*}.go
#done
#mv gobin.go gobin_go

xxd -i axis_obj > axis_obj.h
xxd -i teapot_obj > teapot_obj.h