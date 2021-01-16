(define (problem ijrr_1)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street24 street31 - street
		building0 building15 building16 building27 - building
	)
	(:init
		(atLocation survivor0 street31)
		(atLocation survivor1 building15)
		(onFire building0)
		(onFire building16)
		(underRubble street24)
		(underRubble building27)
		(needsRepair building0)
		(needsRepair building16)
		(needsRepair building27)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street24))
		(not (needsRepair building0))
		(not (needsRepair building16))
		(not (needsRepair building27))
	))
	(:metric minimize total-time)
)
