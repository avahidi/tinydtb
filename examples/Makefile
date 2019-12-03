DIRS=intro dtbdump

all:
	set -e ; for d in $(DIRS); do make -C $$d ; done

clean:
	set -e ; for d in $(DIRS); do make -C $$d clean; done

run:
	set -e ; for d in $(DIRS); do make -C $$d run; done
