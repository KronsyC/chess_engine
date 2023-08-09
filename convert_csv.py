"""
Converts custom output files from the genetic algorithm into csv files
"""

from pathlib import Path


class Agent:
    def __init__(self, score, fields, gen_no):
        self.score = score
        self.gen_no = gen_no
        self.fields = fields
    def csv_encode(self):
        data = ""
        data += f"{self.gen_no},{self.score}"

        for k in self.fields.keys():
            v = self.fields[k]

            for d in v.values():
                data += "," + str(d)
        return data
    def csv_headers(self):
        headers = "generation,fitness"

        for k in self.fields.keys():
            for d in self.fields[k].keys():
                headers += f",{k}.{d}"
        return headers
def grab_agents(fpath, initial_gen = 1):

    data = Path(fpath).read_text()
    entries = data.split("\n---\n")[1:]

    # print(entries[0])

    agents = [e.split("<< Agents >>\n")[1].strip() for e in entries]


    agent_sets = [a.split("\n\n") for a in agents]


    GENERATION = initial_gen
    ret = []
    for ags in agent_sets:
        for a in ags:
            # print(a)
            members = a.split("\n")
            # print(members[1].split(":"))
            score = float(members[1].split(":")[1].strip())
            obj = {}
            for m in members[2:]:
                parts = m.split(";")
                fname = parts[0]
                typ = parts[1]

                if typ == "M":
                    # Multistage
                    [opening, midgame, endgame] = parts[2].split(",")
                    obj[fname] = {
                        "opening": opening,
                        "midgame": midgame,
                        "endgame": endgame
                    }
                else:
                    # Perpiece
                    [king, queen, rook, bishop, knight, pawn] = parts[2].split(",")
                    obj[fname] = {
                        "king": king,
                        "queen": queen,
                        "rook": rook,
                        "bishop": bishop,
                        "knight": knight,
                        "pawn": pawn
                    }
            ag = Agent(score, obj, GENERATION)
            ret.append(ag)
        GENERATION += 1
    return ret



num = 1
paths = []
gen_no = 1
while True:
    path = input(f"Data File Path #{num} (leave empty to submit) > ")

    if not path:
        ALL_AGENTS = []
        for i in range(len(paths)):
            data = grab_agents(paths[i], gen_no)
            for a in data:
                ALL_AGENTS.append(a)
            gen_no = data[-1].gen_no + 1
        print(len(ALL_AGENTS))


        f = open("Training Data.csv", "w")

        f.write(ALL_AGENTS[0].csv_headers() + "\n")
        for a in ALL_AGENTS:
            f.write(a.csv_encode() + "\n")
        f.close()
        break

    else:
        num += 1
        paths.append(path)
        

# print(agent_sets[0][0])