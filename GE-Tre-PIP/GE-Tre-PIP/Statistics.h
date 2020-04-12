#pragma once

struct Statistic {
	long long int cnt_add;
	long long int cnt_multiply;
	long long int cnt_compare;
	long long int cnt_memory;
	long long int cnt_basic_memory;
	long long int cnt_auxiliary_memory;
	Statistic(long long int cnt_add, long  long int cnt_multiply, long  long int cnt_compare, long  long int cnt_memory, long long int cnt_basic_memory, long long int cnt_auxiliary_memory)
		:cnt_add(cnt_add), cnt_memory(cnt_memory), cnt_compare(cnt_compare), cnt_multiply(cnt_multiply) ,
		cnt_basic_memory(cnt_basic_memory),cnt_auxiliary_memory(cnt_auxiliary_memory){};
};