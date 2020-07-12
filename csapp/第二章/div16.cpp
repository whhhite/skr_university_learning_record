int book_div16(int x)
{
	int bias = (x >> 31) & 0xF;//右移31位后，32位上面都是符号位的值
	//如果为非负数，符号位为0，bias变量为0
	//如果为负数，符号位为1，bias变量为0xF，即15
	return (x + bias) >> 4;
}