__author__ = 'thurley'


class htmlwriter:
    # file object

    def __init__(self, filename):
        self.f = open(filename, 'w')

    def add_tag(self, tag, text, f):
        f.write("<" + tag + ">" + text + "</" + tag + ">")

    def add_td(self, text, f):
        add_tag("td", text, f)