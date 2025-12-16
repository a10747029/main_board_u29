# main_board_u29 使用说明

本文档基于 [`User/main.c`](User/main.c)（commit `c8039881bb56b01828ee1265a50be8a62154a3f4`）进行说明。

---

## 1. 功能概述

本程序运行在 GD32F1x0 系列 MCU 上，主要功能如下：

1. 初始化系统时钟、GPIO 及串口、ADC 外设。  
2. 通过 ADC 采集多路电压/电源相关信号（`adc_value[10]`）。  
3. 将采集到的 ADC 数组按固定协议通过 `USART0` 周期性上报给上位机。  
4. （预留）对各路电压进行阈值判断，超出范围时控制蜂鸣器报警（目前相关判断代码被 `#if 0` 屏蔽）。

蜂鸣器使用 `GPIOB PIN2` 控制，默认关闭。

---

## 2. 硬件资源

- MCU：GD32F1x0 系列（需包含 ADC、USART0、USART1、GPIOB）  
- 串口：
  - `USART0`：数据上报接口（协议输出口）
  - `USART1`：用于 `printf` 重定向调试输出（`fputc` 实现）
- GPIO：
  - `GPIOB PIN2`：蜂鸣器控制（高电平使能）
- ADC 通道：程序中使用 `extern uint16_t adc_value[10];` 保存采样结果  
  实际通道与 `adc_value` 下标的映射在 `adc_init()` 中配置（不在本文件中）。

> 说明：ADC 阈值宏定义仅用于说明各路量测信号对应含义，具体缩放关系需结合硬件原理图。

---

## 3. 主流程说明

### 3.1 初始化

`main()` 中的初始化步骤：

1. `rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV2);`  
   - AHB 时钟分频为系统时钟的一半，用于外设运行。

2. `rcu_periph_clock_enable(RCU_GPIOB);`  
   - 打开 GPIOB 时钟。

3. `gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_OSPEED_2MHZ, GPIO_PIN_2);`  
   - 将 `PB2` 配置为 2MHz 推挽输出，用于蜂鸣器控制。

4. `gpio_bit_reset(GPIOB, GPIO_PIN_2);`  
   - 默认关闭蜂鸣器（输出低电平）。

5. `com_init0();`  
   - 串口0（USART0）初始化，具体波特率等参数在其它文件中配置。

6. `adc_init();`  
   - ADC 初始化，配置多通道采样，并将转换结果存入全局数组 `adc_value[10]`。

### 3.2 周期性上报流程

在 `while(1)` 主循环中，程序执行以下操作：

1. **帧头发送**  
   ```c
   usart_data_transmit(USART0, 0xAA);
   while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
   usart_data_transmit(USART0, 0xAA);
   while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
   ```
   - 连续发送两个字节 `0xAA 0xAA` 作为数据帧帧头。

2. **发送 ADC 数据区（20 字节）**

   ```c
   uint8_t *ch = (uint8_t*)adc_value;
   for (i = 0; i < 20; i += 2)
   {    
       usart_data_transmit(USART0, *(ch + i + 1));
       while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
       usart_data_transmit(USART0, *(ch + i));
       while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
   }
   ```

   - `adc_value` 为长度 10 的 `uint16_t` 数组，一共 20 个字节。  
   - 通过 `uint8_t *ch = (uint8_t*)adc_value;` 将其按字节访问。  
   - 每次循环发送一个 `uint16_t`，顺序为：
     - 先发送高字节 `*(ch + i + 1)`  
     - 再发送低字节 `*(ch + i)`  
   - 共发送 10 组数据，即 20 个字节（Big-Endian，高在前低在后）。

3. **发送 CRC 校验字节**

   ```c
   while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
   usart_data_transmit(USART0, crc8(ch, 20));
   ```

   - 使用 `crc8()` 对 `adc_value` 所占的 20 字节计算 8 位 CRC，结果作为 1 字节的校验字段发送。
   - `crc8()` 的多项式等具体实现不在本文件中，需参考对应源文件。

4. **发送结束字节**

   ```c
   while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
   usart_data_transmit(USART0, 0xff);
   ```

   - 数据帧最后发送一个字节 `0xFF`，可视为帧尾。

5. **触发下一次 ADC 转换并延时**

   ```c
   adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
   delay(1000);
   ```

   - 通过软件触发 ADC 规则通道转换。  
   - `delay(1000);` 为简单空转延时，延时时间与 `TIME_PLUS_MS` 和系统主频有关：
     ```c
     #define TIME_PLUS_MS 1000
     void delay(int time)
     {
         int target_time = TIME_PLUS_MS * time;
         while(target_time--);
         return;
     }
     ```
   - 实际“毫秒”并非精确计时，仅大致形成周期。

---

## 4. 协议格式说明

### 4.1 串口参数（建议）

> 具体参数应以 `com_init0()` 实现为准，这里给出典型配置建议供上位机对接：

- 波特率：`115200`（示例，需根据源码确认）
- 数据位：`8`
- 校验：`None`
- 停止位：`1`
- 流控：无

### 4.2 数据帧结构

每一帧数据格式如下：

| 字段        | 长度（字节） | 说明                                     |
|-------------|--------------|------------------------------------------|
| FrameHead   | 2            | 固定为 `0xAA 0xAA`                       |
| ADC_Data    | 20           | 10 个 `uint16_t`，每个高字节在前        |
| CRC8        | 1            | 对 ADC_Data（20 字节）计算的 8 位 CRC   |
| FrameTail   | 1            | 固定为 `0xFF`                            |

总长度：24 字节。

#### 4.2.1 ADC 数据字段映射

程序中定义了相关的电压阈值宏，隐含了各通道的物理含义：

```c
#define ADC_IN0_VCC24_MIN          3217
#define ADC_IN0_VCC24_MAX          3574
#define ADC_IN1_VCC3V3_ULP_MIN     2247 
#define ADC_IN1_VCC3V3_ULP_MAX     2491
#define ADC_IN2_VCC3V3_ULP_MIN     2247 
#define ADC_IN2_VCC3V3_ULP_MAX     2491
#define ADC_IN3_VCC3V3_ULP_MIN     2247 
#define ADC_IN3_VCC3V3_ULP_MAX     2491
#define ADC_IN4_VBAT_MIN           2518
#define ADC_IN4_VBAT_MAX           3438
#define ADC_IN5_VCC12_SYS_MIN      3084
#define ADC_IN5_VCC12_SYS_MAX      3484
#define ADC_IN6_VCC5V0_CORE_MIN    3231
#define ADC_IN6_VCC5V0_CORE_MAX    3588
#define ADC_IN7_VCC5V0_SYS_MIN     3231
#define ADC_IN7_VCC5V0_SYS_MAX     3588
#define ADC_IN8_VCC5V0_SYS_MIN     3231
#define ADC_IN8_VCC5V0_SYS_MAX     3588
#define ADC_IN9_VCC5V0_SYS_MIN     3231
#define ADC_IN9_VCC5V0_SYS_MAX     3588
```

结合这些宏，可以推测 `adc_value` 各通道含义示例（需与实际 `adc_init()` 配置对应）：

| `adc_value` 下标 | 含义（推测）       | 对应阈值宏                    |
|------------------|--------------------|-------------------------------|
| `adc_value[0]`   | VCC 24V            | `ADC_IN0_VCC24_MIN/MAX`       |
| `adc_value[1]`   | VCC 3V3 ULP        | `ADC_IN1_VCC3V3_ULP_MIN/MAX`  |
| `adc_value[2]`   | VCC 3V3 ULP（或其他） | `ADC_IN2_VCC3V3_ULP_MIN/MAX`  |
| `adc_value[3]`   | VCC 3V3 ULP（或其他） | `ADC_IN3_VCC3V3_ULP_MIN/MAX`  |
| `adc_value[4]`   | VBAT               | `ADC_IN4_VBAT_MIN/MAX`        |
| `adc_value[5]`   | VCC 12V_SYS        | `ADC_IN5_VCC12_SYS_MIN/MAX`   |
| `adc_value[6]`   | VCC 5V0 CORE       | `ADC_IN6_VCC5V0_CORE_MIN/MAX` |
| `adc_value[7]`   | VCC 5V0 SYS        | `ADC_IN7_VCC5V0_SYS_MIN/MAX`  |
| `adc_value[8]`   | VCC 5V0 SYS        | `ADC_IN8_VCC5V0_SYS_MIN/MAX`  |
| `adc_value[9]`   | VCC 5V0 SYS        | `ADC_IN9_VCC5V0_SYS_MIN/MAX`  |

> 实际使用中，请严格以硬件设计和 `adc_init()` 的配置映射为准。

#### 4.2.2 字节序说明

每个 ADC 采样值 `adc_value[n]` 为 16 位无符号整数 (`uint16_t`)，在串口发送时采用高字节在前（Big-Endian）：

- 高字节：`(adc_value[n] >> 8) & 0xFF`
- 低字节：`adc_value[n] & 0xFF`

---

## 5. 协议示例（Sample）

以下示例仅为演示格式，具体数值根据实际 ADC 采样结果而定。

### 5.1 假设 ADC 数组数值

假设：

```c
adc_value[0] = 0x0C91;  // 3217
adc_value[1] = 0x08C7;  // 2247
adc_value[2] = 0x09D3;  // 2515
adc_value[3] = 0x0D0C;  // 3340
adc_value[4] = 0x0CB0;  // 3248
adc_value[5] = 0x0C0C;  // 3084
adc_value[6] = 0x0CA0;  // 3232
adc_value[7] = 0x0D00;  // 3328
adc_value[8] = 0x0D20;  // 3360
adc_value[9] = 0x0D40;  // 3392
```

按高字节在前发送，ADC 数据区依次为（仅展示格式，非真实 CRC）：

```text
AA AA
0C 91  08 C7  09 D3  0D 0C  0C B0
0C 0C  0C A0  0D 00  0D 20  0D 40
CRC  FF
```

- `AA AA`：帧头  
- `0C 91 ... 0D 40`：共 20 字节 ADC 数据  
- `CRC`：对上述 20 字节做 `crc8()` 计算的结果（例如假设为 `0x5A`，则为 `5A`）  
- `FF`：帧尾

完整示例（十六进制）：

```text
AA AA 0C 91 08 C7 09 D3 0D 0C 0C B0 0C 0C 0C A0 0D 00 0D 20 0D 40 5A FF
```

> 注：上面的 CRC 值 `5A` 仅示意，请根据实际 `crc8()` 函数实现和数据实时计算。

---

## 6. CRC8 说明

程序调用形式：

```c
uint8_t crc8(uint8_t *data, size_t length);
...
usart_data_transmit(USART0, crc8(ch, 20));
```

- 输入数据：指向 ADC 数据区首地址的指针 `ch`，长度为 20 字节。  
- 输出：对这 20 字节计算出的 CRC8 校验值。  
- 多项式和初始值等细节需查看 `crc8` 函数定义文件。

上位机解析时需 **使用相同的 CRC8 算法**，否则会校验失败。

---

## 7. 报警逻辑（预留说明）

主循环中存在一段被 `#if 0` 包裹的代码，用于电压异常时触发蜂鸣器（PB2）报警：

```c
#if 0
if(0){
    if(((adc_value[0] < ADC_IN0_VCC24_MIN)       || (adc_value[0] > ADC_IN0_VCC24_MAX))       ||
       ((adc_value[1] < ADC_IN1_VCC3V3_ULP_MIN)  || (adc_value[1] > ADC_IN1_VCC3V3_ULP_MAX))  ||
       ((adc_value[2] < ADC_IN4_VBAT_MIN)        || (adc_value[2] > ADC_IN4_VBAT_MAX))        ||
       ((adc_value[3] < ADC_IN5_VCC12_SYS_MIN)   || (adc_value[3] > ADC_IN5_VCC12_SYS_MAX))   ||
       ((adc_value[4] < ADC_IN6_VCC5V0_CORE_MIN) || (adc_value[4] > ADC_IN6_VCC5V0_CORE_MAX)) ||
       ((adc_value[5] < ADC_IN7_VCC5V0_SYS_MIN)  || (adc_value[5] > ADC_IN7_VCC5V0_SYS_MAX)))
    { 
        gpio_bit_set(GPIOB, GPIO_PIN_2);
        //delay(200);
        delay(60);
        gpio_bit_reset(GPIOB, GPIO_PIN_2);
    }
}
#endif
```

若将其启用并修改 `if(0)` 为 `if(1)` 或移除外层 `if(0)`，则逻辑为：

- 只要任意监测通道的 `adc_value` 超出对应阈值范围，就：
  - 拉高 `PB2`（打开蜂鸣器）
  - 延时一段时间
  - 关闭蜂鸣器

目前该部分为**关闭状态**，只保留为后续扩展和文档说明之用。

---

## 8. `printf` 重定向说明

文件末尾实现了 `fputc` 函数，将标准 C 库的 `printf` 输出重定向到 `USART1`：

```c
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART1, (uint8_t)ch);
    while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
    return ch;
}
```

- 任意 `printf("xxx");` 最终会通过 `USART1` 发送到外部。  
- 便于调试及日志输出。

---

## 9. 上位机解析建议

1. 打开对应串口（连接 `USART0`），配置为与 MCU 一致的串口参数。  
2. 接收数据流，寻找连续两个字节 `0xAA 0xAA` 作为一帧起始。  
3. 接收后续 22 字节数据：
   - 20 字节 ADC 数据
   - 1 字节 CRC8
   - 1 字节帧尾
4. 检查最后一个字节是否为 `0xFF`，不是则丢弃此帧。  
5. 对接收的 20 字节 ADC 数据使用相同的 CRC 算法进行校验，与接收的 CRC8 字节比对：
   - 一致：认为本帧有效
   - 不一致：丢弃本帧
6. 将 20 字节拆分为 10 个 16 位数值：
   - `value[n] = (buf[2*n] << 8) | buf[2*n + 1]`
7. 再根据硬件标定关系（电阻分压、参考电压等）换算为真实电压或物理量。

---

## 10. 版本及参考信息

- 仓库：`a10747029/main_board_u29`
- 文件：`User/main.c`  
- Commit：`c8039881bb56b01828ee1265a50be8a62154a3f4`

如需扩展功能（增加通道、修改协议、使能蜂鸣器报警等），建议同步更新本 README 文档，保持协议描述与实际软件版本一致。
