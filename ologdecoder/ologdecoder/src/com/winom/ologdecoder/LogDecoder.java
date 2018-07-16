package com.winom.ologdecoder;

import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class LogDecoder {

    /* 定义magic数*/
    static int LOG_MAGIC_HEAD = 0x41530949;
    static int LOG_MAGIC_END = 0xb303423a;

    /* 密码的长度 */
    static int LOG_ENCODE_LENGTH = 32;
    static SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSSZ", Locale.US);

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("usage: java -jar faceulog.jar FILE_PATH OUT_PATH");
            return;
        }

        FileInputStream inputStream;
        FileOutputStream outputStream = null;

        try {
            inputStream = new FileInputStream(new File(args[0]));
        } catch (FileNotFoundException e) {
            System.out.println("can't open file: " + args[0]);
            return;
        }

        byte[] buffer = new byte[LOG_ENCODE_LENGTH];
        byte[] encodeArr = new byte[LOG_ENCODE_LENGTH];
        int len;
        byte findEncodeArr = 0;
        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        try {
            while ((len = inputStream.read(buffer, 0, LOG_ENCODE_LENGTH)) > 0) {
                if (buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24) == LOG_MAGIC_HEAD) {
                    // 找到头部
                    byte[] temp = new byte[2 * LOG_ENCODE_LENGTH];
                    System.arraycopy(buffer, 0, temp, 0, LOG_ENCODE_LENGTH);
                    inputStream.read(buffer, 0, LOG_ENCODE_LENGTH);
                    System.arraycopy(buffer, 0, temp, LOG_ENCODE_LENGTH, LOG_ENCODE_LENGTH);
                    System.arraycopy(temp, 4, encodeArr, 0, LOG_ENCODE_LENGTH);
                    findEncodeArr = 1;
                    break;
                }
            }

            if (findEncodeArr == 1) {
                try {
                    outputStream = new FileOutputStream(new File(args[1]));
                } catch (FileNotFoundException e) {
                    closeStream(inputStream);
                    System.out.println("can't create file: " + args[1] + e.getMessage());
                    return;
                }

                do {
                    len = inputStream.read(buffer, 0, LOG_ENCODE_LENGTH);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            buffer[i] ^= encodeArr[i];
                        }
                        baos.write(buffer, 0, len);
                        byte[] baosArr = baos.toByteArray();
                        int index = checkLineBreak(baosArr);
                        if (-1 != index) {
                            byte[] lineByte = formatByteTime(baosArr, index);
                            outputStream.write(lineByte);
                            baos.reset();
                            baos.write(baosArr, index + 1, baosArr.length - index - 1);
                        }
                    } else {
                        outputStream.write(baos.toByteArray());
                    }
                } while (len > 0);
            }
        } catch (IOException e) {
            System.out.println("read file content failed, " + e.getMessage());
        } finally {
            closeStream(inputStream);
            closeStream(outputStream);
        }
    }

    static int checkLineBreak(byte[] outputArr) {
        int pos = 0;
        while (pos < outputArr.length) {
            if (outputArr[pos] == 10) {
                return pos;
            } else if ((outputArr[pos] & Integer.parseInt("10000000", 2)) == Integer.parseInt("00000000", 2)) {
                pos++;
            } else if ((outputArr[pos] & Integer.parseInt("11100000", 2)) == Integer.parseInt("11000000", 2)) {
                pos += 2;
            } else if ((outputArr[pos] & Integer.parseInt("11110000", 2)) == Integer.parseInt("11100000", 2)) {
                pos += 3;
            } else if ((outputArr[pos] & Integer.parseInt("11111000", 2)) == Integer.parseInt("11110000", 2)) {
                pos += 4;
            } else if ((outputArr[pos] & Integer.parseInt("11111100", 2)) == Integer.parseInt("11111000", 2)) {
                pos += 5;
            } else if ((outputArr[pos] & Integer.parseInt("11111110", 2)) == Integer.parseInt("11111100", 2)) {
                pos += 6;
            }
        }
        return -1;
    }

    static void closeStream(Closeable closeable) {
        if (null != closeable) {
            try {
                closeable.close();
            } catch (IOException e) {
                System.out.println("close file failed! " + e.getMessage());
            }
        }
    }

    static byte[] formatByteTime(byte[] origArr, int breakPos) {
        int len = breakPos + 1;
        if (!(origArr[4] == '1' && (origArr[5] >= '4' && origArr[5] <= '9') && len > 17)) {
            byte[] lineByte = new byte[breakPos + 1];
            System.arraycopy(origArr, 0, lineByte, 0, lineByte.length);
            return lineByte;
        }

        // 时间转换过后，长度增加了10字节
        byte[] convArr = new byte[len + 10];
        System.arraycopy(origArr, 0, convArr, 0, 4);
        System.arraycopy(origArr, 17, convArr, 27, len - 17);
        long timeStamp = 0;
        for (int i = 4; i < 17; ++i) {
            timeStamp = timeStamp * 10 + origArr[i] - '0';
        }

        String time = simpleDateFormat.format(new Date(timeStamp));
        try {
            byte[] formatTimeArr = time.getBytes("utf-8");
            System.arraycopy(formatTimeArr, 0, convArr, 4, 23);
        } catch (UnsupportedEncodingException e) {
            System.out.println("UnsupportedEncodingException " + e.getMessage());
        }
        return convArr;
    }
}
