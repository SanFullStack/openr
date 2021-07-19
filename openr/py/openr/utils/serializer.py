#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


from openr.utils.consts import Consts
from thrift.util import Serializer


def serialize_thrift_object(thrift_obj, proto_factory=Consts.PROTO_FACTORY):
    """Serialize thrift data to binary blob

    :param thrift_obj: the thrift object
    :param proto_factory: protocol factory, set default as Compact Protocol

    :return: string the serialized thrift payload
    """

    return Serializer.serialize(proto_factory(), thrift_obj)


def deserialize_thrift_object(
    raw_data, thrift_type, proto_factory=Consts.PROTO_FACTORY
):
    """Deserialize thrift data from binary blob

    :param raw_data string: the serialized thrift payload
    :param thrift_type: the thrift type
    :param proto_factory: protocol factory, set default as Compact Protocol

    :return: instance of thrift_type
    """

    resp = thrift_type()
    Serializer.deserialize(proto_factory(), raw_data, resp)
    return resp
