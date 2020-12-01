# 求助]学堂在线9.3练习第二题



大家晚上好，这个题目我在piazza里面找到了陈老师过去的解答，不过还有疑问：

1.算法看得不是很懂，可否请老师们讲解下这个CLOCK置换算法bit 2到底是如何运行地？每访问一次引用计数加一，00加到11后就一直停在11不再增加？

2.循环指针是如何停放地，停在第一个空闲页处？

3.既然选取置换页时是随机选取地，那seed变了缺页次数难道不会受影响？

 

以下是陈老师的回答：

9.3  用clock算法 （bit 2）的执行过程。请看看与你的理解是否有偏差？ 

算法实现请看 https://github.com/chyyuu/ucore_lab/blob/master/related_info/lab3/page-replacement-policy.py

请思考一下，其实现与原理课上讲的是否不同？

 

Access: 0 MISS Left -> [0] <- Right Replaced:- [Hits:0 Misses:1]
Access: 3 MISS Left -> [0, 3] <- Right Replaced:- [Hits:0 Misses:2]
Access: 2 MISS Left -> [0, 3, 2] <- Right Replaced:- [Hits:0 Misses:3]
Access: 0 HIT Left -> [0, 3, 2] <- Right Replaced:- [Hits:1 Misses:3]
Access: 1 MISS Left -> [0, 3, 2, 1] <- Right Replaced:- [Hits:1 Misses:4]
Access: 3 HIT Left -> [0, 3, 2, 1] <- Right Replaced:- [Hits:2 Misses:4]
Access: 4 MISS Left -> [0, 3, 2, 4] <- Right Replaced:1 [Hits:2 Misses:5]
Access: 3 HIT Left -> [0, 3, 2, 4] <- Right Replaced:- [Hits:3 Misses:5]
Access: 1 MISS Left -> [0, 2, 4, 1] <- Right Replaced:3 [Hits:3 Misses:6]
Access: 0 HIT Left -> [0, 2, 4, 1] <- Right Replaced:- [Hits:4 Misses:6]
Access: 3 MISS Left -> [0, 4, 1, 3] <- Right Replaced:2 [Hits:4 Misses:7]
Access: 2 MISS Left -> [0, 1, 3, 2] <- Right Replaced:4 [Hits:4 Misses:8]
Access: 1 HIT Left -> [0, 1, 3, 2] <- Right Replaced:- [Hits:5 Misses:8]
Access: 3 HIT Left -> [0, 1, 3, 2] <- Right Replaced:- [Hits:6 Misses:8]
Access: 4 MISS Left -> [0, 1, 2, 4] <- Right Replaced:3 [Hits:6 Misses:9]