import sys

def validate_bijection(lst):
    """Check if the input list represents a valid bijection."""
    n = len(lst)
    # Check if all elements are unique and within 0..n-1
    if sorted(lst) != list(range(n)):
        return False
    return True

def invert_bijection(lst):
    """Invert the bijection represented by the list."""
    inverse = [0] * len(lst)
    for i, val in enumerate(lst):
        inverse[val] = i
    return inverse

def main():
    if len(sys.argv) != 2:
        print("Usage: python invert_bijection.py '1,0,4,2,3'")
        sys.exit(1)
    
    try:
        # Parse input
        input_list = [int(x.strip()) for x in sys.argv[1].split(',')]
        
        # Validate it's a bijection
        if not validate_bijection(input_list):
            print("Error: Input is not a valid bijection (must be a permutation of 0..n-1)")
            sys.exit(1)
            
        # Compute and print inverse
        inverse = invert_bijection(input_list)
        print("Inverse bijection:")
        print(',\n'.join(map(str, inverse)))
        
    except ValueError:
        print("Error: All elements must be integers")
        sys.exit(1)

if __name__ == "__main__":
    main()