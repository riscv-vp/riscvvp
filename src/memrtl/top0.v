module top0 # (
    parameter DW  = 32
)
(
    input wire                  clk,
    input wire                  rst_n,
    input wire                  up_valid,
    input wire[DW-1:0]          up_data,
    output wire                 up_ready,
    output wire                 down_valid,
    output wire[DW-1:0]         down_data,
    input wire                  down_ready
);

io_block # (.DW(DW), .OPERATION_WIDTH(1), .LAST_WIDTH(1), .UNUSED_WIDTH(6), .ADDR_WIDTH(16), .DATA_WIDTH(8)) io_blk1
(
    .clk(clk),
    .rst_n(rst_n),
    .up_valid  (up_valid),
    .up_data   (up_data),
    .up_ready  (up_ready),
    .down_valid(down_valid),
    .down_data (down_data),
    .down_ready(down_ready)
);

endmodule
