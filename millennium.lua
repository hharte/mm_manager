-- Wireshark Protocol Dissecter for the Nortel Millennium Payphone
--
-- www.github.com/hharte/mm_manager
--
-- Copyright (c) 2020-2022, Howard M. Harte

-- declare the protocol
millennium_proto = Proto("millennium", "Nortel Millennium Payphone (lua)")

-- declare the value strings
local vs_tableid = {
     [0] = "INVALID",
     [1] = "DLOG_MT_CARD_AUTH_REQ",
     [2] = "DLOG_MT_FUNF_CARD_AUTH",
     [3] = "DLOG_MT_AUTH_RESPONSE",
     [4] = "DLOG_MT_CD_CALL_DETAILS",
     [5] = "DLOG_MT_CDR_DETAILS_ACK",
     [6] = "DLOG_MT_MAINT_REQ",
     [7] = "DLOG_MT_ALARM",
     [8] = "DLOG_MT_CALL_IN",
     [9] = "DLOG_MT_CALL_BACK",
    [10] = "DLOG_MT_TERM_STATUS",
    [11] = "DLOG_MT_CD_CALL_STATS",
    [12] = "DLOG_MT_PERF_STATS",
    [13] = "DLOG_MT_END_DATA",
    [14] = "DLOG_MT_TAB_UPD_ACK",
    [15] = "DLOG_MT_MAINT_ACK",
    [16] = "DLOG_MT_ALARM_ACK",
    [17] = "DLOG_MT_TRANS_DATA",
    [18] = "DLOG_MT_TABLE_UPD",
    [19] = "DLOG_MT_CALL_BACK_REQ",
    [20] = "DLOG_MT_TIME_SYNC",
    [21] = "DLOG_MT_NCC_TERM_PARAMS",
    [22] = "DLOG_MT_CARD_TABLE",
    [23] = "DLOG_MT_CARRIER_TABLE",
    [24] = "DLOG_MT_CALLSCRN_UNIVERSAL",
    [25] = "0x19",
    [26] = "DLOG_MT_FCONFIG_OPTS",
    [27] = "DLOG_MT_VIS_PROPTS_L1",
    [28] = "DLOG_MT_VIS_PROPTS_L2",
    [29] = "DLOG_MT_ADVERT_PROMPTS",
    [30] = "DLOG_MT_USER_IF_PARMS",
    [31] = "DLOG_MT_INSTALL_PARAMS",
    [32] = "DLOG_MT_COMM_STAT_PARMS",
    [33] = "DLOG_MT_MODEM_PARMS",
    [34] = "DLOG_MT_CALL_STAT_PARMS",
    [35] = "DLOG_MT_CALL_IN_PARMS",
    [36] = "DLOG_MT_TIME_SYNC_REQ",
    [37] = "DLOG_MT_PERF_STATS_MSG",
    [38] = "DLOG_MT_CASH_BOX_STATUS",
    [39] = "DLOG_MT_ATN_CALL_BACK",
    [40] = "DLOG_MT_ATN_REQ_TYPE_40",
    [41] = "0x29",
    [42] = "DLOG_MT_ATN_REQ_CDR_UPL",
    [43] = "0x2b",
    [44] = "DLOG_MT_ATN_REQ_TAB_UPD",
    [45] = "0x2d",
    [46] = "0x2e",
    [47] = "0x2f",
    [48] = "0x30",
    [49] = "DLOG_MT_ANS_SUP_PARAMS",
    [50] = "DLOG_MT_COIN_VAL_TABLE",
    [51] = "DLOG_MT_CASH_BOX_COLLECTION",
    [52] = "0x34",
    [53] = "DLOG_MT_CALL_DETAILS",
    [54] = "0x36",
    [55] = "DLOG_MT_REP_DIAL_LIST",
    [56] = "DLOG_MT_SUMMARY_CALL_STATS",
    [57] = "DLOG_MT_CARRIER_CALL_STATS",
    [58] = "DLOG_MT_LIMSERV_DATA",
    [59] = "0x3b",
    [60] = "DLOG_MT_SW_VERSION",
    [61] = "DLOG_MT_COIN_CALL_DETAILS",
    [62] = "DLOG_MT_NUM_PLAN_TABLE",
    [63] = "DLOG_MT_RATE_REQUEST",
    [64] = "DLOG_MT_RATE_RESPONSE",
    [65] = "DLOG_MT_AUTH_RESP_CODE",
    [66] = "0x42",
    [67] = "0x43",
    [68] = "DLOG_MT_MDS_FCONFIG",
    [69] = "DLOG_MT_MDS_STATS",
    [70] = "DLOG_MT_MONDEX_DEPOSIT_REC",
    [71] = "DLOG_MT_CARRIER_STATS_EXP",
    [72] = "DLOG_MT_SPARE_TABLE",
    [73] = "DLOG_MT_RATE_TABLE",
    [74] = "DLOG_MT_LCD_TABLE_1",
    [75] = "DLOG_MT_LCD_TABLE_2",
    [76] = "DLOG_MT_LCD_TABLE_3",
    [77] = "DLOG_MT_LCD_TABLE_4",
    [78] = "DLOG_MT_LCD_TABLE_5",
    [79] = "DLOG_MT_LCD_TABLE_6",
    [80] = "DLOG_MT_LCD_TABLE_7",
    [81] = "DLOG_MT_LCD_TABLE_8",
    [82] = "DLOG_MT_QUERY_TERM_ERR",
    [83] = "DLOG_MT_TERM_ERR_REP",
    [84] = "DLOG_MT_SERIAL_NUM",
    [85] = "DLOG_MT_EXP_VIS_PROPTS_L1",
    [86] = "DLOG_MT_EXP_VIS_PROPTS_L2",
    "Table 87", "Table 88", "Table 89",
    [90] = "DLOG_MT_LCD_TABLE_9",
    [91] = "DLOG_MT_LCD_TABLE_10",
    [92] = "DLOG_MT_CALL_SCREEN_LIST",
    [93] = "DLOG_MT_SCARD_PARM_TABLE",
    [94] = "DLOG_MT_CODE_DOWNLOAD",
    "Table 95", "Table 96", "Table 97", "Table 98", "Table 99", "Table 100",
    [101] = "DLOG_MT_COMP_LCD_TABLE_1",
    [102] = "DLOG_MT_COMP_LCD_TABLE_2",
    [103] = "DLOG_MT_COMP_LCD_TABLE_3",
    [104] = "DLOG_MT_COMP_LCD_TABLE_4",
    [105] = "DLOG_MT_COMP_LCD_TABLE_5",
    [106] = "DLOG_MT_COMP_LCD_TABLE_6",
    [107] = "DLOG_MT_COMP_LCD_TABLE_7",
    [108] = "DLOG_MT_COMP_LCD_TABLE_8",
    [109] = "DLOG_MT_COMP_LCD_TABLE_9",
    [110] = "DLOG_MT_COMP_LCD_TABLE_10",
    [111] = "DLOG_MT_COMP_LCD_TABLE_11",
    [112] = "DLOG_MT_COMP_LCD_TABLE_12",
    [113] = "DLOG_MT_COMP_LCD_TABLE_13",
    [114] = "DLOG_MT_COMP_LCD_TABLE_14",
    [115] = "DLOG_MT_COMP_LCD_TABLE_15",
    [134] = "DLOG_MT_CARD_TABLE_EXP",
    [135] = "DLOG_MT_CARRIER_TABLE_EXP",
    [136] = "DLOG_MT_NPA_NXX_TABLE_1",
    [137] = "DLOG_MT_NPA_NXX_TABLE_2",
    [138] = "DLOG_MT_NPA_NXX_TABLE_3",
    [139] = "DLOG_MT_NPA_NXX_TABLE_4",
    [140] = "DLOG_MT_NPA_NXX_TABLE_5",
    [141] = "DLOG_MT_NPA_NXX_TABLE_6",
    [142] = "DLOG_MT_NPA_NXX_TABLE_7",
    [143] = "DLOG_MT_NPA_NXX_TABLE_8",
    [144] = "DLOG_MT_NPA_NXX_TABLE_9",
    [145] = "DLOG_MT_NPA_NXX_TABLE_10",
    [146] = "DLOG_MT_NPA_NXX_TABLE_11",
    [147] = "DLOG_MT_NPA_NXX_TABLE_12",
    [148] = "DLOG_MT_NPA_NXX_TABLE_13",
    [149] = "DLOG_MT_NPA_NXX_TABLE_14",
    [150] = "DLOG_MT_NPA_SBR_TABLE",
    [151] = "DLOG_MT_INTL_SBR_TABLE",
    [152] = "DLOG_MT_DISCOUNT_TABLE",
    [154] = "DLOG_MT_NPA_NXX_TABLE_15",
    [155] = "DLOG_MT_NPA_NXX_TABLE_16",
--    [160] = "DLOG_MT_NPA_NXX_TABLE_15",
--    [161] = "DLOG_MT_NPA_NXX_TABLE_16",
}

local vs_retry = {
    [0] = "",
    [1] = "RETRY"
}

local vs_ack = {
    [0] = "NACK",
    [1] = " ACK"
}

local vs_no_yes = {
    [0] = "No",
    [1] = "Yes"
}

local vs_flags_status = {
    [0] = "Success",
    [1] = "Failure"
}

local vs_start = {
    [2] = "Good",
}

local vs_end = {
    [3] = "Good",
}

local vs_src_direction = {
    [0] = "Manager ",
    [1] = "Terminal"
}

local vs_dst_direction = {
    [0] = "Terminal",
    [1] = "Manager "
}

packet_continuation = {}

-- declare the fields
local f_start = ProtoField.uint8("millennium.start", "Start", base.HEX, vs_start)
local f_flags = ProtoField.uint8("millennium.flags", "Flags", base.HEX)
local f_flags_rxseq = ProtoField.uint8("millennium.flags.rxseq", "RX Sequence", base.DEC, NULL, 0x03)
local f_flags_txseq = ProtoField.uint8("millennium.flags.txseq", "TX Sequence", base.DEC, NULL, 0x03)
local f_flags_retry = ProtoField.uint8("millennium.flags.retry", "Retry", base.DEC, vs_retry, 0x04)
local f_flags_ack = ProtoField.uint8("millennium.flags.ack", "ACK", base.DEC, vs_ack, 0x08)
local f_flags_status = ProtoField.uint8("millennium.flags.status", "Status", base.DEC, vs_flags_status, 0x10)
local f_flags_disconnect = ProtoField.uint8("millennium.flags.disconnect", "Disconnect", base.DEC, vs_no_yes, 0x20)
local f_flags_bits67 = ProtoField.uint8("millennium.flags.bits67", "Reserved", base.DEC, NULL, 0xC0)

local f_len = ProtoField.uint8("millennium.len", "Length", base.DEC)
local f_crc = ProtoField.uint16("millennium.crc", "CRC", base.HEX)
local f_end = ProtoField.uint8("millennium.end", "End", base.HEX, vs_end)
local f_termid = ProtoField.bytes("millennium.data", "Terminal ID")
local f_tableid = ProtoField.uint8("millennium.tableid", "Table ID", base.DEC_HEX, vs_tableid)
local f_src = ProtoField.uint8("millennium.src", "Packet Source", base.DEC, vs_src_direction)
local f_dst = ProtoField.uint8("millennium.dst", "Packet Destination", base.DEC, vs_dst_direction)

millennium_proto.fields = { f_start, f_flags, f_flags_rxseq, f_flags_txseq, f_flags_retry, f_flags_ack, f_flags_status, f_flags_disconnect, f_flags_bits67, f_len, f_crc, f_end, f_termid, f_tableid, f_src, f_dst }

function millennium_proto.dissector(buffer, pinfo, tree)

-- Set the protocol column
    pinfo.cols['protocol'] = "millennium"

    -- create the millennium protocol tree item
    local t_millennium = tree:add(millennium_proto, buffer())
    local offset = 0

    -- Add the header tree item and populate it
    local t_hdr = t_millennium:add(buffer(offset, 3), "Header")
    local start = buffer(offset, 1):uint()
    local direction = 1 -- Terminal to Manager
    if start == 0x82 then
        direction = 0   -- Manager to Terminal
    end

    if direction == 0 then
        pinfo.src = Address.ip('1.1.1.1')
        pinfo.dst = Address.ip('2.2.2.2')
        pinfo.p2p_dir = 0
        start = bit.band(start, 0x7F)   -- remove the pseudo-direction bit.
    else
        pinfo.src = Address.ip('2.2.2.2')
        pinfo.dst = Address.ip('1.1.1.1')
        pinfo.p2p_dir = 1
    end

    t_hdr:add(f_src, direction)
    t_hdr:add(f_dst, direction)
    t_hdr:add(f_start, start)

    local totlen = buffer(offset + 2, 1):uint()
    local crclen = totlen - 2
    local datalen = totlen - 5
    local flags_range = buffer(offset + 1, 1)
    local flags = flags_range:uint()
    local sequence = bit.band(flags, 0x03)
    local t_flags = t_hdr:add(f_flags, flags_range, flags)

    -- If direction == TX and the packet is not 0-length, use txseq
    -- If direction == RX and the packet is 0-length (ACK), use txseq
    -- If direction == RX and the packet is not 0-length, use rxseq
    -- If direction == TX and the packet is 0-length (ACK), use rxseq

    if (direction == 0 and datalen > 0) or (direction == 1 and datalen == 0) then
        t_flags:add(f_flags_txseq, flags_range, flags)
    else
        t_flags:add(f_flags_rxseq, flags_range, flags)
    end

    t_flags:add(f_flags_retry, flags_range, flags)
    t_flags:add(f_flags_ack, flags_range, flags)
    t_flags:add(f_flags_status, flags_range, flags)
    t_flags:add(f_flags_disconnect, flags_range, flags)
    t_flags:add(f_flags_bits67, flags_range, flags)

    t_hdr:add(f_len, buffer(offset + 2, 1))
    offset = offset + 3

    local table_id

    if datalen > 0 then
        table_id = buffer(offset + 5, 1):uint()
        local t_data = t_millennium:add(buffer(offset, datalen), "Message")
        t_data:add(f_termid, buffer(offset, 5))
        t_data:add(f_tableid, buffer(offset + 5, 1))
        if datalen == 250 then
            packet_continuation[pinfo.number] = table_id
        end
        offset = offset + datalen
    end

    local t_trailer = t_millennium:add(buffer(offset, 3), "Trailer")
    local packet_crc = buffer(offset, 2):le_uint()
    t_trailer:add_le(f_crc, buffer(offset, 2))

    local calculated_crc = crc16(buffer(0, crclen), crclen)

    t_trailer:add(f_end, buffer(offset+2, 1))

    -- Set the info column to the name of the function
    pinfo.cols.info = "Seq=" .. sequence .. ": "

    if datalen == 0 then
        if bit.band(flags, 0x20) == 0x20 then
            pinfo.cols.info:append("DISCONNECT")
        else
            if bit.band(flags, 0x08) == 0x08 then
                pinfo.cols.info:append("ACK ")
            else
                pinfo.cols.info:append("NACK ")
            end

            if direction == 0 then
                pinfo.cols.info:append("Terminal ")
            else
                pinfo.cols.info:append("Manager ")
            end
        end
    else
        pinfo.cols.info:append(vs_tableid[table_id] .. ", (datalen=" .. datalen .. " bytes)")
    end

    if calculated_crc ~= packet_crc then
        pinfo.cols.info:append(string.format(" CRC Error: Received 0x%04x, Expected 0x%04x", packet_crc, calculated_crc))
        t_trailer:add_expert_info(PI_CHECKSUM, PI_ERROR, string.format("CRC Error: Calculated 0x%04x, Received 0x%04x", packet_crc, calculated_crc))
    end

end

wtap_encap_table = DissectorTable.get("wtap_encap")
wtap_encap_table:add(wtap.USER0, millennium_proto)

function crc16(data, length)
    sum = 0x0
    local d
    for i = 0, length-1 do
        d = data(i, 1):uint()
        if i == 0 then
            d = bit.band(d, 0x7F)   -- remove the pseudo-direction bit.
        end
        sum = bit.bxor(sum, d)
        for i = 0, 7 do
            if (bit.band(sum, 1) == 0) then
                sum = bit.rshift(sum, 1)
            else
                sum = bit.bxor(bit.rshift(sum, 1), 0xA001)
            end
        end
    end
    return sum
end
