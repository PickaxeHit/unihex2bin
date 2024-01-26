# unihex2bin
Generate binary font from unifont .hex file.
## 文件格式
##### 所有多字节整型皆为小端
1. 文件头（12字节长）
- 标志位：3字节，为'UFB'。
- 版本：1字节，二进制无符号uint8_t。
- 字形数据偏移量：4字节，二进制无符号uint32_t，相对于文件开始的偏移量（目前没啥用，后续可能有用，也可以用于快速跳过）。
- 起始、终止Unicode平面：各1字节，二进制无符号uint8_t。
- 保留：2字节，0xFF
2. 地址表
- 紧随文件头
- 4字节为一个地址，要快速查抄某个Unicode代码点，请将code*4+12，随后读取4字节
- 4字节不全为地址，低26位为实际地址
- 最高位标记字形宽度：0为8px宽，1为16px宽
- 其后1位标记该字形是否属于组合符号：1为是，0为否
- 其后4位为组合符号偏移量：同其他有符号整形一样，最高位为符号位。以2字节为单位，即：如果为-6，那么在渲染它的时候需要将该字形左移12px，叠加在上一个字符上
3. 字形数据
- 根据读到的字形宽度来确定一个字形占的字节数，如果宽度为8则为16字节，16则为32字节
- 先横后纵，先低位后高位，即：左上角的像素位于第一字节的最低位，右下角的像素位于最后一字节的最高位
- 0表示none，1表示dot

## 使用方法
`unihex2bin -i <unifont.hex> -o <output.bin>`


可选参数-p可以指定覆盖平面，如-p 1则指定平面1，-p 0-3则指定平面1-平面3。默认平面0
可选参数-c指定组合字符的偏移，格式为`-c <combining.txt>`
