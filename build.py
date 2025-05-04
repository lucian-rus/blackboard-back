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
    os.system('python tester_rx.py localhost')

def run_tester_rx_no_server():
    # run tester tx
    os.system('python tester_rx.py ' + sys.argv[2])

def run_callable():
    input_args = sys.argv
    for arg in input_args[1:]:
        arg_dict[arg] = None

    if '--no-server' in arg_dict:
        os.chdir(os.path.dirname(__file__) + '/tools/tester')
        os.system("alias python=\"python3\"")

        tester_rx_thread = threading.Thread(target=run_tester_rx_no_server)
        tester_rx_thread.start()
    else:
        execute_build()

        if '-r' in arg_dict:
            backend_thread = threading.Thread(target=run_backend)
            backend_thread.start()

        if '-tx' in arg_dict:
            # waits 2 seconds for server to be properly started
            time.sleep(2)

            os.chdir(os.path.dirname(__file__) + '/tools/tester')
            os.system("alias python=\"python3\"")
            tester_tx_thread = threading.Thread(target=run_tester_tx)
            tester_tx_thread.start()

        if '-rx' in arg_dict:
            # waits 2 seconds for server to be properly started
            time.sleep(2)
            
            os.chdir(os.path.dirname(__file__) + '/tools/tester')
            os.system("alias python=\"python3\"")

            tester_rx_thread = threading.Thread(target=run_tester_rx)
            tester_rx_thread.start()


# function to check if virtual environment is enabled. if not, activate it
def virtual_env_running():
    return 'VIRTUAL_ENV' in os.environ

# @todo: update this to handle venv init
# if not virtual_env_running():
#     os.system('/bin/bash --rcfile ' + os.path.dirname(__file__) + '/tools/scripts/env/bin/activate')
run_callable()
