from random import randint

def generate_ppm(name, width, height):
    with open(name, 'w') as ppm:
        print('P3', file=ppm)
        print(f'{width} {height}', file=ppm)
        print('255', file=ppm)
        for x in range(0, height):
            for y in range(0, width):
                r = randint(125, 175)
                g = 0 # randint(0, 255)
                b = randint(125, 175)
                print(f'{r} {g} {b}', file=ppm)

def main():
    for x in range(2, 8):
        n = pow(2, x)
        file_name = f'{n}_image.ppm'
        print(f'Generating: {file_name}')
        generate_ppm(file_name, n, n)

if __name__ == '__main__':
    main()

