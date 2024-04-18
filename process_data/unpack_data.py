import pickle
import struct

import pandas as pd
import numpy as np

DW_TIME_TO_MS = (1.0 / 499.2e6 / 128.0) * 1000  # 将dw3000时间戳转换为ms
PERIOD = 5  # 周期为ms
offset = 0

with open('../data/2023-09-05-09-53-14.pkl', 'rb') as file:
    data = pickle.load(file)#从这个文件中读取字节流，并将其反序列化成原始的Python对象。
data = pd.DataFrame(data)

# print(data)#603*6
'''
处理溢出问题，使得时间点连续
:param row:
:return:
'''
def process_overflow(row):
    global offset
    if row['diff'] < 0:
        offset += (0x10000000000 * DW_TIME_TO_MS)
    row['sniffer_rx_time'] += offset + PERIOD
    return row



'''
解析下面的结构体
typedef union{
    struct {
    uint8_t rawtime[5];//低5位字节
    uint8_t address; //最高1位字节
    uint16_t seqNumber; //最高2-3位字节
    }__attribute__((packed));
    dwTime_t timestamp; // 8 byte, 后5字节有用，高3字节未使用
} Body_Unit_t;
'''
def parse_body_unit(data):

    # 检查输入数据长度是否为8字节
    if len(data) != 8:
        raise ValueError("Data must be exactly 8 bytes long.")

    # 解析数据Body_Unit_t中的数据
    rawtime = data[:5]  # 前5字节
    address = struct.unpack('B', data[5:6])[0]  # 第6字节
    seqNumber = struct.unpack('H', data[6:8])[0]  # 最后2字节

    # 假设dwTime_t是一个8字节整数，我们仅使用后5字节
    # 由于高3字节未使用，我们需要正确处理字节顺序
    dwTime_t = struct.unpack('<Q', b'\x00\x00\x00' + data[3:8])[0]  # 使用小端字节序，并补充3个零字节到最高位
    dwTime_ms = (dwTime_t & 0xffffffffff)* DW_TIME_TO_MS
    #返回值是一个字典类型的数据
    return {
        'a': address,
        'b': seqNumber,
        'c': dwTime_ms
    }



'''
typedef struct
{
    uint16_t srcAddress;               // 2 byte
    uint16_t msgSequence;              // 2 byte
    Timestamp_Tuple_t lastTxTimestamp; // 10 byte
    short velocity;                    // 2 byte cm/s
    /*--1添加--*/
    short velocityXInWorld; // 2 byte cm/s 在世界坐标系下的速度（不是基于机体坐标系的速度）
    short velocityYInWorld; // 2 byte cm/s 在世界坐标系下的速度（不是基于机体坐标系的速度）
    float gyroZ;            // 4 byte rad/s
    uint16_t positionZ;     // 2 byte cm/s
    bool keep_flying;       // 无人机的飞行状态，布尔值占用一个字节 
    int8_t stage;           // 飞行阶段
    // bool isAlreadyTakeoff;  // 是否起飞
    /*--1添加--*/
    uint16_t msgLength;                               // 2 byte
    uint16_t filter;                                  // 16 bits bloom filter
} __attribute__((packed)) Ranging_Message_Header_t; // 20 byte


typedef struct
{
  dwTime_t timestamp;                        // 8 byte
  uint16_t seqNumber;                        // 2 byte
} __attribute__((packed)) Timestamp_Tuple_t;
'''
def parse_header(header_orginal_data):

    format_string = '<H H QH h h h f H ? b H H'
    header_unpacked_data = struct.unpack(format_string, header_orginal_data)
    header = {
        'd':header_unpacked_data[0],
        'e':header_unpacked_data[1],
        'f':header_unpacked_data[2],
        'g':header_unpacked_data[3],
    }

    return header#返回值为一个字典



def unpacked(x: pd.DataFrame):
    binData = x['bin_data']
    #解析bin_data之中的header部分
    x['header'] = parse_header(binData[0:32])#如果 'header' 键不存在，则此操作将创建一个新的键 'header' 并赋予它指定的值。
    #不包含第32个字节

    #解析bin_data之中的body部分
    base = 32#头部32个字节
    body_unit_size = 8#body部分每个8字节
    len_bin_data = len(binData)
    i = 0
    while base + body_unit_size <= len_bin_data:
        i = i + 1
        body_unit = parse_body_unit(binData[base:base + body_unit_size])#数据切片
        base += body_unit_size
        x['body_unit_' + str(i)] = body_unit#为DataFrame添加新的键，str(i)将一个整数转换为字符串的形式

    print(x)
    print("--------------------------------------------------------------------------------------------------------")
    return x


data.loc[:, 'sniffer_rx_time'] = data.loc[:, 'sniffer_rx_time'] & 0xffffffffff  # 得到正确的时间戳，这里:表示选择所有行，'sniffer_rx_time'指定了列名。
data.loc[:, 'sniffer_rx_time'] = data.loc[:, 'sniffer_rx_time'] * DW_TIME_TO_MS  # 换算成ms
data.loc[:, 'diff'] = data.loc[:, 'sniffer_rx_time'] - data.loc[:, 'sniffer_rx_time'].shift(1)#：表示所有的行，shift(1)表示下移一行
data.loc[:, 'diff'].fillna(0, inplace=True)# DataFrame 中的 'diff' 列中的空值（NaN）填充为零，并将填充后的结果直接应用到原始 DataFrame 


data = data.apply(process_overflow, axis=1)
data: pd.DataFrame
data.reset_index(inplace=True)#简化索引的作用
data.loc[:, 'sniffer_rx_time'] = data.loc[:, 'sniffer_rx_time'] - data.loc[0, 'sniffer_rx_time']

START_POINT = 0  # 起始数据点
STOP_POINT = len(data) - 1  # 最后数据点
start_rv_time = data.loc[START_POINT, 'sniffer_rx_time']
stop_rv_time = data.loc[STOP_POINT, 'sniffer_rx_time']
data = data.loc[START_POINT:STOP_POINT].apply(unpacked, axis=1)#包含解析头文件和body部分


#从data DataFrame中移除名为'magic'、'bin_data'和'diff'的列
drop_coloums = ['magic', 'bin_data','diff','index','msg_len']
# print(data.columns)
data = data.drop(drop_coloums,axis=1)

print(data.columns)

data.to_csv("test1.csv",header=False,index=False)
#不包含列头、不包含索引
