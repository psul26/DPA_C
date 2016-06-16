#!/usr/bin/env python

import sys
import array
import copy

class AttackTrace:
    trace_plaintext_size_bytes = 16
    trace_ciphertext_size_bytes = 16
    trace_num_points = 3253

    def __init__(self):
        self.plaintext = array.array('B')
        self.ciphertext = array.array('B')
        self.samples = array.array('h')

    def read(self, fd):
        self.plaintext = array.array('B')
        self.ciphertext = array.array('B')
        self.samples = array.array('h')

        # Plaintext (16 bytes)
        self.plaintext.fromfile(fd, self.trace_plaintext_size_bytes)

        # Ciphertext (16 bytes)
        self.ciphertext.fromfile(fd, self.trace_ciphertext_size_bytes)

        # Samples (3253 * signed 16-bit values)
        self.samples.fromfile(fd, self.trace_num_points)

        # All data read are little-endian
        if sys.byteorder == 'big':
            self.samples.byteswap()


class AttackPartialResult:
    trace_key_size_bytes = 16

    def __init__(self, subkey_num):
        assert ((subkey_num > 0) & (subkey_num <= 10))
        self.subkey_num = subkey_num
        self.bytes = []

        for i in range(self.trace_key_size_bytes):
            tmp = array.array('B')
            for j in range(256):
                tmp.append(j)

            self.bytes.append(tmp)


    def write(self, fd):
        # Subkey num (1 byte)
        fd.write(chr(self.subkey_num))

        # Bytes
        tmp_bytes = None

        if sys.byteorder == 'big':
            tmp_bytes = copy.deepcopy(self.bytes)
            for i in range(self.trace_key_size_bytes):
                tmp_bytes[i].byteswap()
        else:
            tmp_bytes = self.bytes

        for i in range(self.trace_key_size_bytes):
            tmp_bytes[i].tofile(fd)

        fd.flush()


def read_num_traces(fd):
    tmp = fd.read(2)
    num_traces = ord(tmp[0]) + ord(tmp[1]) * 256

    return num_traces


def send_start(fd):
    fd.write("\x0A\x2E\x0A")
    fd.flush()

