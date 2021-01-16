(define (problem ijrr_11)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street6 street17 - street
		building5 building11 building23 building27 - building
	)
	(:init
		(atLocation survivor0 street6)
		(atLocation survivor1 building5)
		(onFire building11)
		(onFire building27)
		(underRubble street17)
		(underRubble building23)
		(needsRepair building11)
		(needsRepair building23)
		(needsRepair building27)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street17))
		(not (needsRepair building11))
		(not (needsRepair building23))
		(not (needsRepair building27))
	))
	(:metric minimize total-time)
)
