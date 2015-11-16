


import re
import random

# Weights from a random forest of 200 trees mtry = 5 and inputs unordered.
weights1 = [0.0037813963299895,0.00453537966729657,0.00547984266355995,0.00577618239133293,0.00574091091228884,0.00541267264146298,0.00540771229734257,0.00524096920321665,0.0053455548898104,0.00562373987963483,0.00586268023134048,0.00569128668765888,0.00584187991546711,0.00550976425270342,0.00628925638898491,0.00643541204620045,0.00634406126602654,0.00680128524299023,0.00682394438900327,0.0069334407203749,0.00808825878919715,0.00841589880673103,0.00813259334679372,0.0081192456623318,0.00806581139376066,0.00860139770824678,0.00816328860505539,0.00418742873434464,0.00663998995601344,0.00900529974055795,0.00891275418773033,0.00931579596595359,0.00927815750159607,0.00936409021045959,0.00886895287748104,0.00922051247019659,0.00939554992565565,0.00982678942104175,0.0105218732987235,0.0105594691636793,0.0096855959915534,0.0114519070178804,0.0108127527103159,0.0112852202171243,0.0147659933403828,0.0151906387113408,0.015719099330909,0.0191227739443914,0.0225165228874521,0.024997607342377,0.0270650479691187,0.023370268492546,0.0224090195474083,0.0240020547498655,0.00419096637018462,0.00818974298796706,0.0100368820328186,0.01154062080706,0.0105166091459385,0.0107011300820368,0.012490635034848,0.0107710317155169,0.0107054078887248,0.0109112443295786,0.0101091699056681,0.0099189779463452,0.00993061350886865,0.00968801789185531,0.0090086490120319,0.00945628322080372,0.0113803453085804,0.0108619222796584,0.0132749961217174,0.0106356835245653,0.0121168482134842,0.013702335727513,0.0152476667461544,0.0135742860162066,0.0151229504543719,0.0171242508218751,0.0157791105604194,0.00406893653280412,0.00561507830465168,0.00672341085344986,0.00700806423106569,0.00654049301512736,0.00657621743204033,0.00600848329436182,0.00571304802989978,0.00565798132582796,0.00582951046922475,0.00566625382046896,0.00541987738524299,0.00565944618101242,0.00533094033130259,0.00534621680643941,0.0054889871840254,0.00581348607737261,0.00587238315162554,0.00565504081663703,0.00568228814964275,0.00560900502577329,0.00590012911143425,0.00562204812123684,0.0052997395715782,0.00497759954651579,0.00499140024569429,0.0049825252958505]


class DataProcessor:


    def __init__ (self, file_name):
        # Read file into an array.
        self.data_array = []
        self.pos = 0 # Used to keep track of out position in the file.
        for line in open(file_name, 'r'):
            self.data_array +=  [line.rstrip()]

    # Read the file and convert the next particle into a particle object.
    def next_particle(self):
        # Find the position in the file where the next particle begins.
        while True:
            line = self.data_array[self.pos]
            if line == 'ELECTRON' or line == 'PION':
                particle = Particle(line)
                tracklet = []
                self.pos += 1
                break
            self.pos += 1
        # Read each line of the file.
        while True:
            line = self.data_array[self.pos]
            if '|' in line:
                # Remove pad information
                line = re.sub(r'\d*:\d*:\d*','',line)
                # Replace white spaces with ','
                line = re.sub(' +',',',line)
                # Remove trailing sum, split into an list, convert to int and add it to the list of tracklets.
                tracklet += [map(int, line[1:line.rfind('|')-1].split(','))]
            elif '-' in line:
                # We've found the begining of new tracklet.
                particle.add_tracklet(tracklet)
                tracklet = []
            else:
                return particle
                break
            self.pos += 1


    # Return True if there are no more particle to be read from the file.
    def is_empty(self):
        for i in range(self.pos, len(self.data_array)):
            line = self.data_array[i]
            if line == 'ELECTRON' or line == 'PION':
                return False
        return True

    # Count the occurance of the particle type in the file ('ELECTRON' or 'PION')
    def count(self, particle_type):
        count = 0
        for line in self.data_array:
            if line == particle_type.upper():
                count += 1
        return count



class Particle:


    def __init__ (self, particle_type):
        self.particle_type = particle_type
        self.tracklets = []
        if particle_type == "ELECTRON":
            self.particle_id = 1
        else:
            self.particle_id = 0

    def add_tracklet(self, tracklet):
        self.tracklets += [tracklet]

    def __str__ (self):
        to_return = self.particle_type + '\n'
        for tracklet in self.tracklets:
            for column in tracklet:
                to_return += str(column) + '\n'
            to_return += '\n'
        return to_return


    # Add the total of the ADC values for each tracklet of the particle.
    def summation(self):
        tracklet_totals =[]
        for tracklet in self.tracklets:
            total = 0
            for column in tracklet:
                for value in column:
                    total+=value
            tracklet_totals += [total]
        mean = sum(tracklet_totals) / len(tracklet_totals)
        while(len(tracklet_totals)!= 6):
            tracklet_totals += [mean]
        to_return = ''
        for total in tracklet_totals:
            to_return  += str(total) + ' '
        return to_return + str(self.particle_id)

    # Add the total of the ADC values for each tracklet of the particle.
    def smeared_summation(self, smear_factor):
        tracklet_totals =[]
        for tracklet in self.tracklets:
            total = 0
            for column in tracklet:
                for value in column:
                    total+=value
            tracklet_totals += [total]
        mean = sum(tracklet_totals) / len(tracklet_totals)
        while(len(tracklet_totals)!= 6):
            tracklet_totals += [mean]
        to_return = ''
        for total in tracklet_totals:
            smear_direction = round(random.uniform(0,1))
            if smear_direction == 0:
                smear_direction = -1
            #print smear_direction
            to_return  += str(total * (1 + smear_factor * smear_direction)) + ' '
        return to_return + str(self.particle_id)

    def weighted_summation(self, weights):
        to_return = ''
        tracklet_totals = []
        for tracklet in self.tracklets:
            tracklet_ADC_values = []
            for column in tracklet:
                for value in column:
                    tracklet_ADC_values += [value]
            total = 0
            for i in range(0, len(tracklet_ADC_values)):
                #print tracklet_ADC_values[i]
                #print weights[i]
                #if weights[i] <= 0.009:
                #    weights[i] = 0
                if weights[i] < 0:
                    weights[i] = 0
                total += tracklet_ADC_values[i] * weights[i] * 100000
            tracklet_totals += [total]

        mean = sum(tracklet_totals) / len(tracklet_totals)
        while(len(tracklet_totals)!= 6):
            tracklet_totals += [mean]
        to_return = ''
        for total in tracklet_totals:
            to_return  += str(total) + ' '
        return to_return + str(self.particle_id)

    # Returns, for each tracklet, a list of
    # ADC values in the original order.
    # Returned as a string.
    def ADC_values(self):
        to_return = ''
        for tracklet in self.tracklets:
            for column in tracklet:
                for value in column:
                    to_return += str(value) + ' '
            to_return += str(self.particle_id) + '\n'
        return to_return

    # Return a list conatianing the average of ADC
    # values across columns for each time bin.
    # Returned as a string.
    def avg_time_bins (self):
        time_bins = [0] * 27
        to_return = ''
        for tracklet in self.tracklets:
            for column in tracklet:
                for i in range(0, len(column)):
                    time_bins[i] += column[i]
        # Average the totals calulated above.
        # This needs to be done because some
        # particles have less than 6 tracklets.
        for time_bin in time_bins:
            time_bin = time_bin / len(self.tracklets)
            to_return += str(time_bin) + ' '
        return to_return + str(self.particle_id)


    # Return True if any of the ADC values are too big.
    def too_big(self):
        for tracklet in self.tracklets:
            for column in tracklet:
                for value in column:
                    # The values are meant to only be 10-bits.
                    # Values which have reached the boundary are probably wrong.
                    if value >= 1023:
                        return True
        return False

    def func(self):
        to_return = ''
        overall_total = [0] * 8
        for tracklet in self.tracklets:
            totals = [0] * 8
            for column in tracklet:
                totals[0] += sum(column[0:4])
                totals[1] += sum(column[4:8])
                totals[2] += sum(column[8:12])
                totals[3] += sum(column[12:16])
                totals[4] += sum(column[16:19])
                totals[5] += sum(column[19:22])
                totals[6] += sum(column[22:25])
                totals[7] += sum(column[25:28])
            for i in range(0,8):
                overall_total[i] += totals[i]
            for i in range(0,8):
                to_return += str(totals[i]) + ' '
            to_return += str(self.particle_id) + '\n'
        for i in range(0,8):
            overall_total[i] /= len(self.tracklets)
        for i in range(0, 6 - len(self.tracklets)):
            for i in range(0,8):
                to_return += str(overall_total[i]) + ' '
            to_return += str(self.particle_id) +'\n'
        return to_return

def reduce (file_name, max_size = 255.0):
        f = open(file_name, 'r')
        # Get the maximum and minimum value in the file.
        max_value = 0
        min_value = 1000000
        for line in f:
            for value in line.split(' '):
                if (not '\n' in value) and float(value) > max_value:
                    max_value = round(float(value))
                elif (not '\n' in value) and float(value) < min_value:
                    min_value = round(float(value))
        #print max_value
        #print min_value
        f = open(file_name, 'r')

        factor = max_size / max_value
        #print factor
        for line in f:
            to_print = ''
            for value in line.split(' '):
                if '\n' in value:
                    to_print += value.strip()
                else:

                    to_print += str(int(round(float(value) * factor))) + ' '
            print to_print


def remove_electrons(data_processor):
    num_pions = data_processor.count("PION")
    #print num_pions
    num_electron = round(num_pions / 10)
    while (not data_p.is_empty()):
        particle = data_p.next_particle()
        if particle.too_big():
            continue
        if num_electron >= 0 and particle.particle_id == 1:
            num_electron -= 1
            print particle.summation()
        elif particle.particle_id == 0:
            print particle.summation()




if __name__ == "__main__":

    

    data_p = DataProcessor('BaseData.txt')
    #remove_electrons(data_p)
    count = 0
    while (not data_p.is_empty()):
        particle = data_p.next_particle()

        if particle.too_big():
            continue
        
        


    print count


