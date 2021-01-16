(define (problem ijrr_4)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street7 street44 - street
		building4 building31 building33 building34 - building
	)
	(:init
		(atLocation survivor0 street44)
		(atLocation survivor1 building31)
		(onFire building4)
		(onFire building34)
		(underRubble street7)
		(underRubble building33)
		(needsRepair building4)
		(needsRepair building33)
		(needsRepair building34)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street7))
		(not (needsRepair building4))
		(not (needsRepair building33))
		(not (needsRepair building34))
	))
	(:metric minimize total-time)
)
