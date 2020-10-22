import os

for filename in os.listdir('.'):
    if filename.endswith('.json'):
        parts = filename.split('_')
        alpha = float(parts[2])
        parts[2] = "{0:0.2f}".format(alpha)
        os.rename(filename, '_'.join(parts)) 
