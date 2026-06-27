PYTHON ?= python

.PHONY: run show-config install-server install-contents install-all mount-contents start-server deploy deploy-windows deploy-linux clean test

run:
	$(PYTHON) src/gmbr_ds.py

show-config:
	$(PYTHON) src/gmbr_ds.py --action show-config

install-server:
	$(PYTHON) src/gmbr_ds.py --action install-server

install-contents:
	$(PYTHON) src/gmbr_ds.py --action install-contents

install-all:
	$(PYTHON) src/gmbr_ds.py --action install-all

mount-contents:
	$(PYTHON) src/gmbr_ds.py --action mount-contents

start-server:
	$(PYTHON) src/gmbr_ds.py --action start-server

deploy:
	$(PYTHON) scripts/deploy.py

deploy-windows:
	$(PYTHON) scripts/deploy.py --system Windows

deploy-linux:
	$(PYTHON) scripts/deploy.py --system Linux

test:
	$(PYTHON) -m unittest discover -s tests

clean:
	$(PYTHON) -c "import pathlib, shutil; [shutil.rmtree(pathlib.Path(p), ignore_errors=True) for p in ('build', 'release', 'Servidor', '__pycache__', 'src/__pycache__', 'tests/__pycache__')]"
