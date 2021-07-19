#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import random
import string
import unittest
from builtins import range

from openr.Types import ttypes as openr_types
from openr.utils import serializer
from thrift.protocol.TJSONProtocol import TJSONProtocolFactory


class TestSerialization(unittest.TestCase):
    def test_reverse_equality(self):
        for _ in range(100):
            thrift_obj = openr_types.PrefixDatabase()
            random_string = "".join(random.choice(string.digits) for _ in range(10))
            thrift_obj.thisNodeName = random_string
            raw_msg = serializer.serialize_thrift_object(thrift_obj)
            recovered_obj = serializer.deserialize_thrift_object(
                raw_msg, openr_types.PrefixDatabase
            )
            self.assertEqual(thrift_obj, recovered_obj)

        for _ in range(100):
            thrift_obj = openr_types.PrefixDatabase()
            random_string = "".join(random.choice(string.digits) for _ in range(10))
            thrift_obj.thisNodeName = random_string
            raw_msg = serializer.serialize_thrift_object(
                thrift_obj, TJSONProtocolFactory
            )
            recovered_obj = serializer.deserialize_thrift_object(
                raw_msg, openr_types.PrefixDatabase, TJSONProtocolFactory
            )
            self.assertEqual(thrift_obj, recovered_obj)

    def test_thrifttype_sensitivity(self):
        thrift_obj = openr_types.PrefixDatabase()
        thrift_obj.thisNodeName = "some node"
        raw_msg = serializer.serialize_thrift_object(thrift_obj)
        recovered_obj = serializer.deserialize_thrift_object(
            raw_msg, openr_types.PrefixEntry
        )
        self.assertTrue(thrift_obj != recovered_obj)

    def test_exception_handling(self):
        thrift_obj = openr_types.PrefixDatabase()
        thrift_obj.thisNodeName = "some node"
        raw_msg = serializer.serialize_thrift_object(thrift_obj)
        # should raise exception due to inconsistency of protocol factor
        with self.assertRaises(Exception):
            serializer.deserialize_thrift_object(
                raw_msg, openr_types.PrefixDatabase, TJSONProtocolFactory
            )
