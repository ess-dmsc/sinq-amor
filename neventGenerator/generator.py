import os, fnmatch

class Generator:

    # This list has to be in the favorite order: if multiple
    # executables are find the first will be used
    exe = None;

    def find_impl(self,pattern, path):
        for root, dirs, files in os.walk(path):
            for name in files:
                if fnmatch.fnmatch(name, pattern):
                    return os.path.join(root, name)
        return None

    def find(self,path=None):
        paths = [ './build/',
                  '.',
                  '/opt/amor/simfiles/',
                  '/opt/software/sinq-amorsim/',
                  '/opt/amor/simfiles/']
        if path is not None:
            paths.insert(0,path)

        for p in paths:
            exe = self.find_impl('AMORgenerator', p)
            if exe :
                self.exe = exe
                return exe

    def validate(self, param) :
        pos = [i for i, j in enumerate(param) if j == '-b']
        if pos != []:
            return True
        return False

def main() :
    g = Generator()
    g.find()
    print g.exe

if __name__ == "__main__":
    main()
