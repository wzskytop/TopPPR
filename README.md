# TopPPR
## 准备工作
将准备好的数据[file].txt放入dataset文件夹内，并在ppr-answer文件夹下创建同名的子文件夹[file]
##编译
执行compile.sh即可
## TopPPR查询步骤
（1）首先需要对数据集生成随机query点，方法见gen_query.sh，其中-d表示数据名字，-n表示生成的query点数
（2）其次需要生成数据的GroundTruth，方法见gen_groundtruth.sh
 (3) 最后运行TopPPR即可，方法见query.sh，需要注意的是-e参数需要根据不同数据大小手动调整，会影响ForwardSearch的运行时间。
