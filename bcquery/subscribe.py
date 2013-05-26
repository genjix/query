import bitcoin
import deserialize
import threading
import struct
import zmq
import Queue

class SubscribeContext(zmq.Context):
    pass

class BaseSubscribe(threading.Thread):

    daemon = True

    def __init__(self, context, server, port):
        super(BaseSubscribe, self).__init__()
        self.subscriber = context.socket(zmq.SUB)
        self.subscriber.connect("tcp://%s:%s" % (server, port))
        self.subscriber.setsockopt(zmq.SUBSCRIBE, "")
        self.queue = Queue.Queue()
        self.start()

    # Options for timeout are None, 0 or a positive number.
    def pop(self, timeout=0):
        block = True
        if timeout == 0:
            block = False
        try:
            return self.queue.get(block, timeout)
        except Queue.Empty:
            return None

class BlockSubscribe(BaseSubscribe):

    def __init__(self, context, server="localhost", port=5563):
        super(BlockSubscribe, self).__init__(context, server, port)

    def run(self):
        while True:
            message = self.subscriber.recv()
            depth = struct.unpack("<L", message)[0]
            message = self.subscriber.recv()
            block = bitcoin.parse_block(message)
            self.queue.put((depth, block))

class TransactionSubscribe(BaseSubscribe):

    def __init__(self, context, server="localhost", port=5564):
        super(TransactionSubscribe, self).__init__(context, server, port)

    def run(self):
        while True:
            message = self.subscriber.recv()
            tx = bitcoin.parse_transaction(message)
            self.queue.put(tx)

