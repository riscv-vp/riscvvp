module io_block #(parameter DW = 32, parameter OPERATION_WIDTH = 1, parameter LAST_WIDTH = 1, parameter UNUSED_WIDTH = 6, parameter ADDR_WIDTH = 16, parameter DATA_WIDTH = 8)
(
input                   clk,
input                   rst_n,
input                   up_valid,
input  [(DW-1):0]       up_data,
output                  up_ready,
output                  down_valid,
output [(DW-1):0]       down_data,
input                   down_ready
);

wire en;
wire write_or_read;
wire ram_we;

wire [(OPERATION_WIDTH - 1):0] operation_bus_in, operation_bus_out_d;
wire [(LAST_WIDTH - 1):0] last_bus_in, last_bus_out_d;
wire [(UNUSED_WIDTH - 1):0] unused_bus_in, unused_bus_out_d;
wire [(ADDR_WIDTH - 1):0] addr_bus_in, addr_bus_out_d;
wire [(DATA_WIDTH - 1):0] data_bus_in, data_memread, data_bus_out_d;

reg down_valid_q;
reg [(OPERATION_WIDTH - 1):0] operation_bus_out_q;
reg [(LAST_WIDTH - 1):0] last_bus_out_q;
reg [(UNUSED_WIDTH - 1):0] unused_bus_out_q;
reg [(ADDR_WIDTH - 1):0] addr_bus_out_q;
reg [(DATA_WIDTH - 1):0] data_bus_out_q;

assign operation_bus_in = up_data[(DATA_WIDTH + ADDR_WIDTH + UNUSED_WIDTH + LAST_WIDTH)+:OPERATION_WIDTH];
assign last_bus_in = up_data[(DATA_WIDTH + ADDR_WIDTH + UNUSED_WIDTH)+:LAST_WIDTH];
assign unused_bus_in = up_data[(DATA_WIDTH + ADDR_WIDTH)+:UNUSED_WIDTH];
assign addr_bus_in = up_data[(DATA_WIDTH)+:ADDR_WIDTH];
assign data_bus_in = up_data[0+:DATA_WIDTH];

assign operation_bus_out_d = operation_bus_in;
assign last_bus_out_d = last_bus_in;
assign unused_bus_out_d = unused_bus_in;
assign addr_bus_out_d = addr_bus_in;
assign data_bus_out_d = (write_or_read) ? data_bus_in : data_memread;

assign en = up_valid & down_ready;
assign write_or_read = operation_bus_in[0];
assign ram_we = en & write_or_read;

//Pseudo Dual port RAM with async read (aka REGISTER FILE)
reg [DATA_WIDTH-1:0] ram[2**ADDR_WIDTH-1:0];

always @ (posedge clk)
    begin
        if (ram_we)
        ram[addr_bus_in] <= data_bus_in;
    end
assign data_memread = ram[addr_bus_in];

always @(posedge clk)
    if (down_ready)
    begin
        operation_bus_out_q <= operation_bus_out_d;
        last_bus_out_q <= last_bus_out_d;
        unused_bus_out_q <= unused_bus_out_d;
        addr_bus_out_q <= addr_bus_out_d;
        data_bus_out_q <= data_bus_out_d;
    end

always @(posedge clk)
    if (~rst_n)
        down_valid_q <= 1'b0;
    else if (down_ready)
        down_valid_q <= up_valid;

assign down_valid = down_valid_q;
assign down_data = {operation_bus_out_q, last_bus_out_q, unused_bus_out_q, addr_bus_out_q, data_bus_out_q};
assign up_ready = down_ready;

endmodule
