Test flow:
•	Write 256B pattern data to W77Q flash section 1 (called mtd1 “user” partition)
•	Read the 256B pattern data from the W77Q flash section 1
•	Verify that the read data match to written data
•	erase written sector (4KB)
•	print a PASS/FAIL banner

Note: Section 0 (16MB) is reserved for U-Boot.
