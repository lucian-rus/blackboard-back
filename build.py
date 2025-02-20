import os
import sys
import threading
import time

arg_dict = {}

def execute_build():
    os.chdir(os.path.dirname(__file__) + '/build')

    # execute build
    os.system('cmake ..')
    os.system('make')

def run_backend():
    # run server
    os.chdir(os.path.dirname(__file__) + '/build/output')
    os.system('./backend')

def run_tester_tx():
    # run tester tx
    os.system('python tester_tx.py')

def run_tester_rx():
    # run tester tx
    os.system('python tester_rx.py')

def run_callable():
    execute_build()

    input_args = sys.argv
    for arg in input_args[1:]:
        arg_dict[arg] = None

    if '-r' in arg_dict:
        backend_thread = threading.Thread(target=run_backend)
        backend_thread.start()

    if '-t' in arg_dict:
        # waits 2 seconds for server to be properly started
        time.sleep(2)

        os.chdir(os.path.dirname(__file__) + '/tools/tester')
        os.system("alias python=\"python3\"")
        tester_rx_thread = threading.Thread(target=run_tester_rx)
        tester_tx_thread = threading.Thread(target=run_tester_tx)
        tester_rx_thread.start()
        tester_tx_thread.start()


run_callable()
