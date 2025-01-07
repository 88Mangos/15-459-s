import numpy as np

def flip_coin(p, n=1):
    """
    Simulates flipping a coin with probability `p` of landing heads.

    Parameters:
    - p (float): Probability of the coin landing heads (0 <= p <= 1).
    - n (int): Number of flips to simulate (default is 1).

    Returns:
    - list: A list of outcomes where True represents heads and False represents tails.
    """
    if not (0 <= p <= 1):
        raise ValueError("Probability p must be between 0 and 1.")

    # Simulate n flips with the given probability
    flips = np.random.rand(n) < p

    return flips

# Example usage
if __name__ == "__main__":
    p = 0.2  # Probability of 1
    n = 10**8   # Number of coin flips
    for i in range(20):
        results = flip_coin(p, n)
        print(f"phat_{i+1} = {np.sum(results) / 10**8}")
    
    
    
