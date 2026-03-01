### DRAFT DOCUMENT
## Testbench to Verify TLM2.0 Verilated memory model:

This is the RTL of the memory block with Pin2Queue and Queue2Pins transactors:
![Image](./media/rtl_ram_block.jpg)

Testbench is forming vector that is passed with TLM2.0 transaction to RTL model.
In the TLM2.0 memory model there is "b_transport" that processes this transaction placing each data beat into REQ queue (blocking write) and waiting (blocking read) from the RESP queue.
This way there are no two in-flight beats in the REQ-RESP pipeline.
In order to verify propagation of backpressure from pin2queue transactor via RTL all the way to queue2pin we need to be able to poke data in REQ queue and peek it from the RESP queue using non blocking read and write.
This way we shall pass delays with which poke and peak will we attempted. This is done by testbench using TLM2.0 extension that contains these delays.
If extension is present the model b_transport will use non blocking mode to work with REQ and RESP queue to process that data burst.
Each TLM2.0 transaction to RTL model is followed by the same transaction to the functional model of the memory and then two TLM2.0 responses are compared.

## Transactions flow:
This diagram shows transaction flow where stages to Verilated RTL model are marked with red numbers and stages to functional model marked with blue numbers.
![Image](./media/tb_with_memrtl.jpg)
