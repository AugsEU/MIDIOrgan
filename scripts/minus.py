import sys

def minus_five(numbers_str):
    numbers = [int(num) for num in numbers_str.split(",")]
    results = [str(num - 5) for num in numbers]
    return ",".join(results)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python minus.py \"num1,num2,num3\"")
        sys.exit(1)
    
    input_str = sys.argv[1]
    try:
        output = minus_five(input_str)
        print(output)
    except ValueError:
        print("Error: Input must be comma-separated numbers")