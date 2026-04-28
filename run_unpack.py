import sys
from unipacker.core import UnpackerEngine

def unpack(input_file, output_file):
    engine = UnpackerEngine(input_file, output_file)
    engine.unpacker.unpack()

if __name__ == "__main__":
    unpack(sys.argv[1], sys.argv[2])
